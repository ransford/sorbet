# typed: true

# rubocop:disable all

module OuterModule
  extend T::Sig

  sig {params(name: String).returns(Integer)}
  def module_method(name)
    name.length
  end

  sig {params(name: String).returns(Integer)}
  def self.module_self_method(name)
    name.length
  end

  sig {returns T.untyped}
  def module_usages
    s = "Foo"
    [
      module_method(s),
      # ^ hover: def module_method(name)
      OuterModule.module_self_method(s),
      #           ^ hover: def self.module_self_method(name)
      OuterModule::module_self_method(s),
      #            ^ hover: def self.module_self_method(name)
    ]
  end

  class InnerClass
    extend T::Sig
    sig {params(name: String).void}
    def initialize(name)
      @name = name
    end

    sig {params(name: String).returns(Integer)}
    def self.class_self_method(name)
      name.length
    end
  
    sig {void}
    def no_args_return_void
      # no args
    end

    sig {returns(Integer)}
    def no_args_return_value
      "no args".length
    end

    sig {params(fname: String, lname: String).returns(String)}
    def positional_args(fname, lname)
      "#{fname}:#{lname}"
    end

    sig {params(fname: String, lname: String).returns(String)}
    def keyword_args_no_defaults(fname:, lname:)
      "#{fname}:#{lname}"
    end

    sig {params(fname: String, lname: String).returns(String)}
    def keyword_args_with_defaults(fname: "Jane", lname: "Doe")
      "#{fname}:#{lname}"
    end

    sig {params(names: String).returns(String)}
    def splat_args(*names)
      names.join ':'
    end

    sig {params(blk: T.proc.params(arg0: Integer).returns(String)).returns(String)}
    def block_args(&blk)
      blk.call(1)
    end

    sig {params(pos: String, splat: String, req_key: String, opt_key: String, blk: T.proc.params(a: String).returns(String)).returns(String)}
    def multiple_arg_types(pos, *splat, req_key:, opt_key: "Jane", &blk)
      "#{pos} #{req_key} #{opt_key} #{splat.join ':'} #{blk.call(pos)}"
    end

    sig {returns T.untyped}
    def class_usages
      s = "Foo"
      qualified = InnerClass.new(s)
                             # ^ hover: def initialize(name)
      [
        no_args_return_void,
        # ^ hover: def no_args_return_void
        no_args_return_value,
        # ^ hover: def no_args_return_value
        positional_args(s, s),
        # ^ hover: def positional_args(fname, lname)
        keyword_args_no_defaults(fname: s, lname: s),
        # ^ hover: def keyword_args_no_defaults(fname:, lname:)
        keyword_args_with_defaults,
        # ^ hover: def keyword_args_with_defaults(fname:…, lname:…)
        splat_args,
        # ^ hover: def splat_args(*names)
        splat_args(s, s, s, s, s),
        # ^ hover: def splat_args(*names)
        block_args {|x| "blk#{x}"},
        # ^ hover: def block_args(&blk)
        multiple_arg_types(s, s, s, s, req_key: s) {|x| x},
        # ^ hover: def multiple_arg_types(pos, *splat, req_key:, opt_key:…, &blk)
        InnerClass::class_self_method(s),
        #           ^ hover: def self.class_self_method(name)
        qualified.no_args_return_void,
        #         ^ hover: def no_args_return_void
        qualified.no_args_return_value,
        #         ^ hover: def no_args_return_value
        qualified.positional_args(s, s),
        #         ^ hover: def positional_args(fname, lname)
        qualified.keyword_args_no_defaults(fname: s, lname: s),
        #         ^ hover: def keyword_args_no_defaults(fname:, lname:)
        qualified.keyword_args_with_defaults(fname: s, lname: s),
        #         ^ hover: def keyword_args_with_defaults(fname:…, lname:…)
        qualified.splat_args(s, s, s, s, s),
        #         ^ hover: def splat_args(*names)
        qualified.multiple_arg_types(s, s, s, s, req_key: s) {|x| x},
        #         ^ hover: def multiple_arg_types(pos, *splat, req_key:, opt_key:…, &blk)
      ]
    end
  end
end
