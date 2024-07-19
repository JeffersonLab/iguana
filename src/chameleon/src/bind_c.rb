require_relative 'chameleon'

class Bind_c < Chameleon

  ##################################################################################

  def initialize(out_name='', algo_name='')
    super(out_name, algo_name, 'C bindings')
    @out.puts <<~END_CODE
      // C functions to provide action function bindings for Fortran

      #{deterrence_banner 'c'}

      #include "#{@algo_header}"
      #include "Bindings.h"

      // clang-format off
      namespace iguana::bindings::#{@algo_name} {
        extern "C" {
    END_CODE
  end

  ##################################################################################

  def bind(spec)
    @action_spec = spec
    ftn_name = get_spec @action_spec, 'name'
    ftn_type = get_spec @action_spec, 'type'
    ftn_name_fortran = "iguana_#{@algo_name.downcase.gsub /::/, '_'}_#{ftn_name.downcase}"
    ftn_name_c = "#{ftn_name_fortran}_"
    verbose " - bind #{ftn_type} function #{@algo_name}::#{ftn_name}"
    check_function_type "#{@algo_name}::#{ftn_name}", ftn_type

    # lists of "parts" we need for the binding function
    docstring_param_list = ['[in] algo_idx the algorithm index']
    par_list             = ['algo_idx_t* algo_idx']
    call_arg_list        = []
    assignment_list      = []

    @result_var = 'result' # the name of the result of an action function call

    # functions to generate lists of parts
    def gen_parts(key)
      parts_spec = get_spec @action_spec, key
      parts_spec.map do |var|
        name = ''
        if key == 'outputs' and parts_spec.size == 1 # true if it's a single, anonymous output
          name = @result_var
        else
          name = get_spec var, 'name'
          error "don't name your variable '#{name}', since this name is reserved" if name == @result_var
        end
        type = get_spec var, 'type'
        cast = get_spec var, 'cast', false
        yield name, type, cast
      end
        .compact
    end

    # function to generate parameter docstring
    def gen_docstring_param(key, type_docstring)
      gen_parts(key) do |name, type|
        if name == @result_var
          "[#{type_docstring}] #{name} the resulting value"
        else
          "[#{type_docstring}] #{name}"
        end
      end
    end

    # function to generate C function parameters
    def gen_ftn_params(key)
      gen_parts(key) do |name, type, cast|
        "#{cast.nil? ? type : cast}* #{name}"
      end
    end

    # function to generate C++ function arguments
    def gen_call_args(key)
      gen_parts(key) do |name, type, cast|
        return nil if name == @result_var
        cast.nil? ? "*#{name}" : "#{type}(*#{name})"
      end
    end

    # function to generate result assignments
    def gen_assignments(key)
      gen_parts(key) do |name, type|
        if name == @result_var
          "*#{name} = out;"
        else
          "*#{name} = out.#{name};"
        end
      end
    end

    # generate parts lists
    case ftn_type
    when 'filter'
      docstring_param_list += gen_docstring_param 'inputs', 'in'
      docstring_param_list << "[in,out] #{@result_var} the filter return value; if this value is already set, the result will be the `AND` of the initial value and this filter"
      par_list += gen_ftn_params 'inputs'
      par_list << "bool* #{@result_var}"
      call_arg_list += gen_call_args 'inputs'
      assignment_list << "*#{@result_var} = *#{@result_var} && out;"
    when 'creator'
      docstring_param_list += gen_docstring_param 'inputs', 'in'
      docstring_param_list += gen_docstring_param 'outputs', 'out'
      par_list += gen_ftn_params 'inputs'
      par_list += gen_ftn_params 'outputs'
      call_arg_list += gen_call_args 'inputs'
      assignment_list += gen_assignments 'outputs'
    end

    # generate code
    @out.puts <<~END_CODE.gsub(/^/,'  ')

      /// This function is for Fortan usage, called as
      /// ```fortran
      ///       call #{ftn_name_fortran}(algo_idx, ...params...)
      /// ```
      /// It binds to the following C++ action function, wherein you can find its documentation:
      /// - `iguana::#{@algo_name}::#{ftn_name}`
      #{docstring_param_list.map{|s|"/// @param #{s}"}.join "\n"}
      void #{ftn_name_c}(
        #{par_list.join ', '})
      {
        auto out = dynamic_cast<iguana::#{@algo_name}*>(iguana_get_algo_(algo_idx))->#{ftn_name}(
          #{call_arg_list.join ', '});
        #{assignment_list.join "\n  "}
      }
    END_CODE

  end

  ##################################################################################

  def close
    @out.puts <<~END_CODE
        }
      }
      // clang-format on
    END_CODE
    @out.close
  end

end
