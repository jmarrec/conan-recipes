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
  void ruby_gc_set_params(void);
  void Init_enc(void);
  void Init_ext(void);
  // void Init_extra_exts(void);  // Customization point
  void rb_call_builtin_inits(void);
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
  ruby_gc_set_params();
  ruby_init_loadpath();

  Init_enc();
  // rb_enc_set_default_internal(rb_enc_from_encoding(rb_utf8_encoding()));
  // rb_enc_set_default_external(rb_enc_from_encoding(rb_utf8_encoding()));

  Init_ext(); /* load statically linked extensions before rubygems */
  // Init_extra_exts();
  rb_call_builtin_inits();
#else
  ruby_init_loadpath();
#endif

  rb_eval_string(R"(
       begin
         puts "I can correctly load call Dir builtin"
         puts "Dir.glob=#{Dir.glob('*')}"
       rescue Exception => e
         puts
         puts "#{e.class}: #{e.message}"
         puts "Backtrace:\n\t" + e.backtrace.join("\n\t")
         raise
       end
     )");

  rb_eval_string(R"(
       begin
         (require 'bigdecimal')
         puts "I can correctly load one of the extension gems - bigdecimal"
       rescue Exception => e
         puts
         puts "#{e.class}: #{e.message}"
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
