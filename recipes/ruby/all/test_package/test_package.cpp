#include <ruby.h>
#include <ruby/version.h>
#include <iostream>
#include <string>

// when --static-linked-ext is used, ruby defines EXTSTATIC as 1
#if defined(EXTSTATIC) && EXTSTATIC
#  define RUBY_STATIC_LINKED_EXT2
#else
#  undef RUBY_STATIC_LINKED_EXT2
#endif

#ifdef RUBY_STATIC_RUBY
extern "C"
{
  // static void Init_builtin_prelude(void);
  // void Init_builtin_gem_prelude(void);

  void Init_builtin_array(void);
  void Init_builtin_ast(void);
  void Init_builtin_dir(void);
  void Init_builtin_gc(void);
  void Init_builtin_io(void);
  void Init_builtin_kernel(void);
  void Init_builtin_marshal(void);
  void Init_builtin_nilclass(void);
  void Init_builtin_numeric(void);
  void Init_builtin_pack(void);
  void Init_builtin_ractor(void);
  void Init_builtin_timev(void);
  void Init_builtin_trace_point(void);
  void Init_builtin_warning(void);
  void Init_builtin_symbol(void);
  void Init_builtin_thread_sync(void);
  // void Init_builtin_yjit(void);
}
#endif

int main(int argc, char* argv[]) {
  ruby_sysinit(&argc, &argv);
  RUBY_INIT_STACK;
  ruby_init();

  std::cout << "RUBY_API_VERSION_MAJOR=" << RUBY_API_VERSION_MAJOR << ", RUBY_API_VERSION_MINOR=" << RUBY_API_VERSION_MINOR
            << ", RUBY_API_VERSION_TEENY=" << RUBY_API_VERSION_TEENY << '\n';
  rb_eval_string("puts 'Hello, ruby!'");

#ifdef RUBY_STATIC_RUBY
  rb_eval_string("puts 'Ruby itself is statically linked'");
#else
  rb_eval_string("puts 'Ruby itself is dynamically linked'");
#endif

#ifdef RUBY_STATIC_LINKED_EXT
  rb_eval_string("puts 'Ruby has statically linked extensions'");
#else
  rb_eval_string("puts 'Ruby has dynamically linked extensions'");
#endif

#ifdef RUBY_STATIC_LINKED_EXT2
  rb_eval_string("puts 'Ruby has statically linked extensions (EXTSTATIC)'");
#else
  rb_eval_string("puts 'Ruby has dynamically linked extensions (EXTSTATIC)'");
#endif

#ifdef RUBY_STATIC_RUBY

  Init_builtin_gc();
  Init_builtin_ractor();
  Init_builtin_numeric();
  Init_builtin_io();
  Init_builtin_dir();
  Init_builtin_ast();
  Init_builtin_trace_point();
  Init_builtin_pack();
  Init_builtin_warning();
  Init_builtin_array();
  Init_builtin_kernel();
#  if RUBY_API_VERSION_MAJOR == 3 && RUBY_API_VERSION_MINOR > 1
  Init_builtin_symbol();
#  endif
  Init_builtin_timev();
#  if RUBY_API_VERSION_MAJOR == 3 && RUBY_API_VERSION_MINOR > 1
  Init_builtin_thread_sync();
#  endif
  // Init_builtin_yjit();
  Init_builtin_nilclass();
  Init_builtin_marshal();

  // Init_builtin_prelude();
  // Init_builtin_gem_prelude();

  rb_provide("bigdecimal");
  rb_provide("bigdecimal.so");
#else
  ruby_init_loadpath();
#endif

  rb_eval_string(R"(
       begin
         (require 'bigdecimal')
         puts "I can correctly load one of the extension gems - bigdecimal"
         puts "Dir.glob=#{Dir.glob('*')}"
       rescue Exception => e
         puts
         puts "Error: #{e.message}"
         puts "Backtrace:\n\t" + e.backtrace.join("\n\t")
         raise
       end
     )");

  /*
  rb_eval_string(R"(
       def absolute_path(path)
         absolute_path = File.dirname(caller_locations(1,1)[0].path) + '/' + path
         return absolute_path
       end

       begin
         absolute_path = absolute_path('/test')
         absolute_path = '/Users/julien/Software/Others/conan/conan-center-index/recipes/ruby/all/test_package/test'
         puts "Trying to require #{absolute_path}"
         (require absolute_path)
         puts "I can correctly load the test.rb file"
       rescue Exception => e
         puts
         puts "Error: #{e.message}"
         puts "Backtrace:\n\t" + e.backtrace.join("\n\t")
         raise
       end
     )");*/

  rb_require(TEST_RB_ABS_PATH);
  // rb_require("/Users/julien/Software/Others/conan/conan-center-index/recipes/ruby/all/test_package/test");

  ruby_finalize();

  return EXIT_SUCCESS;
}

extern "C"
{
  int rb_hasFile(const char* t_filename) {
    // TODO Consider expanding this to use the path which we have artificially defined in embedded_help.rb
    std::string expandedName = std::string(":/ruby/2.7.0/") + std::string(t_filename) + ".rb";
    std::cout << "rb_hasFile called: " << expandedName << std::endl;
    return 0;
  }

  int rb_require_embedded(const char* t_filename) {
    std::string require_script = R"(require ')" + std::string(t_filename) + R"(')";
    std::cout << "require_script called: " << require_script << std::endl;
    return 0;
  }
}
