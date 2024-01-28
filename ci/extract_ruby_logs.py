import argparse
import os
import re
import zipfile
from pathlib import Path, PurePosixPath, PureWindowsPath
from typing import List, Optional
from rich.console import Console


def get_build_folder(content: str) -> Optional[Path]:
    """Find the location of the build folder."""
    RE_BUILD_FOLDER = re.compile("ruby.*: Build folder (.*)")
    if m := RE_BUILD_FOLDER.search(content):
        return Path(m.groups()[0])


def find_interesting_logs(build_folder: Path) -> List[Path]:
    """Return the paths to the mkmf logs, config.status/log, and Makefiles."""
    mkmf_logs = list(build_folder.glob("**/mkmf.log"))
    makefiles = list(build_folder.glob("**/Makefile"))
    config_files = [fp for x in ["config.status", "config.log"] if (fp := (build_folder / x)).is_file()]
    all_files = mkmf_logs + makefiles + config_files
    return all_files


def zip_logs(all_logs: List[Path], zip_path: Path = Path("logs.zip")):
    """Zip the logs."""
    with zipfile.ZipFile(zip_path, "w", compression=zipfile.ZIP_LZMA) as zf:
        for mkmf_log in all_logs:
            zf.write(mkmf_log, mkmf_log.relative_to(build_folder))


# RE_EXT_FAILED = re.compile(r'\*\*\* Following extensions are not compiled.*?\*\*\*', re.DOTALL)
RE_EXT_FAILED = re.compile(r"\*\*\* Following extensions are not compiled:\s(.*?):(.*?)\*\*\*", re.DOTALL)
RE_LOG_MKMF = re.compile(r"Check (ext/.*/mkmf.log) for more details.")


class FailedExtension:
    """Helper class for convenience"""

    def __init__(self, name: str, lines=List[str]):
        self.name = name
        self.ori_lines = lines
        self.mkmf_log = None
        diagnostics = []
        for line in lines:
            line = line.strip()
            if line == "Could not be configured. It will not be installed.":
                continue
            if m := RE_LOG_MKMF.search(line):
                self.mkmf_log = m.groups()[0]
            else:
                diagnostics.append(line)
        self.diagnostics = "\n".join(diagnostics)

    def to_markdown(self, build_folder: Path) -> str:
        fp = build_folder / self.mkmf_log
        assert fp.is_file()
        content = fp.read_text()[:1000]

        return f"""
## {self.name}

{self.diagnostics}

<details>

<summary>**{self.mkmf_log}**</summary>

```
{content}
```
</details>

"""

    def __repr__(self):
        return f"{self.name}: {self.mkmf_log}\n{self.diagnostics}"

    def __rich__(self):
        return f"[red bold]{self.name}[/]: [yellow bold]{self.mkmf_log}[/]\n{self.diagnostics}"


def get_failed_extensions(content) -> List[FailedExtension]:
    """Check for failed extensions."""
    failed_infos = {}
    for ext, msg in RE_EXT_FAILED.findall(content):
        lines = [x.strip() for x in msg.strip().splitlines()]
        if ext not in failed_infos:
            failed_infos[ext] = lines
        else:
            for line in lines:
                if line not in failed_infos[ext]:
                    failed_infos[ext].append(line)
    failed_exts = []
    for ext, lines in failed_infos.items():
        failed_exts.append(FailedExtension(name=ext, lines=lines))
    return failed_exts


def validate_file(arg) -> Path:
    """Throw in argparse if not a valid file."""
    if (filepath := Path(arg)).is_file():
        return filepath
    else:
        raise FileNotFoundError(arg)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Extract logs from ruby build.")
    parser.add_argument(
        "build_log_file", type=validate_file, help="Path to file where the conan build log was captured"
    )
    parser.add_argument(
        "-z",
        "--zip-file",
        type=Path,
        default=Path("logs.zip"),
        required=False,
        metavar="ZIP_FILE",
        help="Where to store the zip file",
    )
    args = parser.parse_args()
    # print(args)
    # exit(0)
    content = Path(args.build_log_file).read_text()

    console = Console()
    build_folder = get_build_folder(content)
    if build_folder is None:
        console.print("[yellow bold]Build folder not found, was the package actually built?[/]")
        exit(0)
    console.print(f"[green]Found Build folder[/]: {str(build_folder)}")

    all_logs = find_interesting_logs(build_folder=build_folder)
    zip_logs(all_logs=all_logs, zip_path=args.zip_file)
    size_mb = args.zip_file.stat().st_size / (1024**2)
    print(f"Zipped the logs to {args.zip_file.absolute()} ({size_mb:.2f} MB)")

    failed_exts = get_failed_extensions(content)
    if not failed_exts:
        console.print("[green bold]No failed extensions[/]")
        exit(0)

    console.print(f"[red bold]There are {len(failed_exts)} failed extensions[/]")
    for failed_ext in failed_exts:
        print("=" * 80)
        console.print(failed_ext)
        if 'RUNNER_DEBUG' in os.environ:
            mkmf_log = build_folder / failed_ext.mkmf_log
            print(mkmf_log.read_text())
        if 'GITHUB_STEP_SUMMARY' in os.environ:
            with open(os.environ["GITHUB_STEP_SUMMARY"], "a") as f:
                f.write(f"\n{failed_ext.to_markdown(build_folder=build_folder)}")
    exit(1)
