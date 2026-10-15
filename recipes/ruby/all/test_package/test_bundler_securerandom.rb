require "fileutils"
require "open3"
require "tmpdir"

Dir.mktmpdir("ruby-bundler-securerandom") do |gem_home|
  gemspec_path = File.join(gem_home, "specifications", "securerandom-0.4.1.gemspec")
  library_path = File.join(gem_home, "gems", "securerandom-0.4.1", "lib", "securerandom.rb")
  gemfile_path = File.join(gem_home, "Gemfile")
  lockfile_path = "#{gemfile_path}.lock"

  FileUtils.mkdir_p(File.dirname(gemspec_path))
  FileUtils.mkdir_p(File.dirname(library_path))
  File.write(gemspec_path, <<~GEMSPEC)
    Gem::Specification.new do |spec|
      spec.name = "securerandom"
      spec.version = "0.4.1"
      spec.summary = "Bundler activation regression fixture"
      spec.authors = ["OpenStudio"]
      spec.files = ["lib/securerandom.rb"]
      spec.require_paths = ["lib"]
    end
  GEMSPEC
  File.write(library_path, <<~RUBY)
    module SecureRandom
      VERSION = "0.4.1"
    end
  RUBY
  File.write(gemfile_path, <<~GEMFILE)
    source "https://rubygems.org"
    gem "securerandom", "= 0.4.1"
  GEMFILE
  File.write(lockfile_path, <<~LOCKFILE)
    GEM
      remote: https://rubygems.org/
      specs:
        securerandom (0.4.1)

    PLATFORMS
      ruby

    DEPENDENCIES
      securerandom (= 0.4.1)

    BUNDLED WITH
       2.4.10
  LOCKFILE

  gem_path = ([gem_home] + Gem.path).uniq.join(File::PATH_SEPARATOR)
  env = {
    "GEM_HOME" => gem_home,
    "GEM_PATH" => gem_path,
    "BUNDLE_GEMFILE" => gemfile_path,
    "BUNDLE_FROZEN" => "true",
    "BUNDLE_DISABLE_VERSION_CHECK" => "true"
  }
  stdout, stderr, status = Open3.capture3(
    env,
    "bundle", "exec", "ruby", "-e",
    'require "securerandom"; abort "wrong securerandom version: #{SecureRandom::VERSION}" unless SecureRandom::VERSION == "0.4.1"'
  )

  unless status.success?
    warn stdout
    warn stderr
    abort "Bundler failed to activate securerandom 0.4.1"
  end
end
