require_relative 'generator'

class Bind_c < Generator

  ##################################################################################

  def initialize(out_name='', algo_name='')
    super(out_name, algo_name, description: 'C bindings', generator_name: __FILE__)
    @out.puts <<~END_CODE
      /// @file #{File.basename out_name}
      /// @brief C functions to provide `iguana::#{@algo_name}` action function bindings for Fortran

      #{deterrence_banner 'c'}

      #include "#{@algo_header}"
      #include "Bindings.h"

      namespace iguana::bindings::#{@algo_name.split('::').first} {
        extern "C" {
    END_CODE
  end

  ##################################################################################

  def bind(spec)

    # get function name and type
    @action_spec = spec
    @ftn_type = get_spec @action_spec, 'type'
    ftn_name = get_spec @action_spec, 'name'
    ftn_name_fortran = "iguana_#{@algo_name.downcase.gsub /::/, '_'}_#{ftn_name.downcase}"
    ftn_name_c = "#{ftn_name_fortran}_"
    verbose " - bind #{@ftn_type} function #{@algo_name}::#{ftn_name}"
    check_function_type "#{@algo_name}::#{ftn_name}", @ftn_type

    # function to generate parameter docstrings
    def gen_docstring_params(key, in_or_out)
      map_spec_params(@action_spec, key) do |name, type, cast|
        if name == RESULT_VAR
          "[#{in_or_out}] #{name} the resulting value"
        else
          "[#{in_or_out}] #{name}"
        end
      end
    end

    # function to generate C function parameters
    def gen_ftn_params(key)
      map_spec_params(@action_spec, key) do |name, type, cast|
        "#{cast.empty? ? type : cast}* #{name}"
      end
    end

    # function to generate C++ function arguments
    def gen_call_args(key)
      map_spec_params(@action_spec, key) do |name, type, cast|
        return nil if name == RESULT_VAR
        cast.empty? ? "*#{name}" : "#{type}(*#{name})"
      end
    end

    # function to generate result assignments
    def gen_assignments(key)
      map_spec_params(@action_spec, key) do |name, type, cast|
        out_var = name == RESULT_VAR ?
          'out' :
          "out.#{name.sub /^#{RESULT_VAR}_/, ''}"
        out_var = "#{cast}(#{out_var})" unless cast.empty?
        "*#{name} = #{out_var};"
      end
    end

    # generate parts lists
    docstring_param_list = ['[in] algo_idx the algorithm index']
    par_list             = ['algo_idx_t* algo_idx']
    call_arg_list        = []
    assignment_list      = []
    case @ftn_type
    when 'filter'
      docstring_param_list += gen_docstring_params 'inputs', 'in'
      docstring_param_list << "[in,out] #{RESULT_VAR} the filter return value; if this value is already set, the result will be the `AND` of the initial value and this filter"
      par_list += gen_ftn_params 'inputs'
      par_list << "bool* #{RESULT_VAR}"
      call_arg_list += gen_call_args 'inputs'
      assignment_list << "*#{RESULT_VAR} = *#{RESULT_VAR} && out;"
    when 'creator', 'transformer'
      docstring_param_list += gen_docstring_params 'inputs', 'in'
      docstring_param_list += gen_docstring_params 'outputs', 'out'
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
    END_CODE
    @out.close
  end

end
