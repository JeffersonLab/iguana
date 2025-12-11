require_relative 'generator'

class Bind_c < Generator

  ##################################################################################

  def initialize(out_name='', algo_name='')
    super(out_name, algo_name, description: 'C bindings', generator_name: __FILE__)
    @out.puts <<~END_CODE
      /// @file #{File.basename out_name}
      /// @brief C functions to provide `iguana::#{@algo_name}` action function bindings for Fortran

      #{deterrence_banner 'c'}

      #include "iguana/algorithms/#{@algo_header}"
      #include "iguana/bindings/Bindings.h"

      namespace iguana::bindings::#{@algo_name.split('::').first} {
        extern "C" {
    END_CODE
  end

  ##################################################################################

  def bind(spec)

    # get function name and type
    @action_spec = spec
    @ftn_type = get_spec @action_spec, 'type'
    @ftn_name = get_spec @action_spec, 'name'
    @ftn_rank = get_spec @action_spec, 'rank'
    ftn_name_fortran = "iguana_#{@algo_name.downcase.gsub /::/, '_'}_#{@ftn_name.downcase}"
    ftn_name_fortran += 'vec' if @ftn_rank == 'vector' # append 'vec' for vector action functions
    ftn_name_c = "#{ftn_name_fortran}_"
    verbose " - bind #{@ftn_type} function #{@algo_name}::#{@ftn_name}"
    check_function_type "#{@algo_name}::#{@ftn_name}", @ftn_type

    # function to generate parameter docstrings
    def gen_docstring_params(key, in_or_out)
      map_spec_params(@action_spec, key) do |name, type, cast, dimension|
        code_str_list = []
        if name == RESULT_VAR
          code_str_list << "[#{in_or_out}] #{name} the resulting value"
        else
          code_str_list << "[#{in_or_out}] #{name}"
        end
        if dimension == 1
          code_str_list << "[#{in_or_out}] #{name}__size the size of `#{name}`"
        end
        code_str_list
      end.flatten
    end

    # function to generate C function parameters
    def gen_ftn_params(key)
      map_spec_params(@action_spec, key) do |name, type, cast, dimension|
        case dimension
        when 0
          [ "#{cast.empty? ? type : cast}* #{name}" ]
        when 1
          [
            "#{cast.empty? ? type : cast}* #{name}",
            "int* #{name}__size",
          ]
        end
      end.flatten
    end

    # function to generate conversions from C-array to C++ std::vector
    def gen_arr2vec(key)
      map_spec_params(@action_spec, key) do |name, type, cast, dimension|
        if dimension == 1
          type_in     = type
          type_out    = cast.empty? ? type_in : cast
          name_array  = name
          name_vector = "#{name_array}__vector"
          elem        = "#{name_array}[i]"
          vector_type = type_out=='bool' ? 'std::deque' : 'std::vector' # use `deque<bool>` insead of forbidden `vector<bool>`
          code_str_list = [
            "#{vector_type}<#{type_out}> #{name_vector}(#{name_array}, #{name_array} + *#{name_array}__size);"
          ]
          if DEBUG
            code_str_list << "fmt::print(\"[C] #{name_array} = [{}]\\n\", fmt::join(#{name_vector}, \", \"));"
          end
          code_str_list
        end
      end.flatten
    end

    # function to generate C++ function arguments
    def gen_call_args(key)
      map_spec_params(@action_spec, key) do |name, type, cast, dimension|
        return nil if name == RESULT_VAR
        case dimension
        when 0
          cast.empty? ? "*#{name}" : "#{type}(*#{name})"
        when 1
          "#{name}__vector"
        end
      end
    end

    # function to generate result assignments
    def gen_assignments(key)
      map_spec_params(@action_spec, key) do |name, type, cast, dimension|
        code_str_list = []
        out_var = name == RESULT_VAR ?
          'out' :
          "out.#{name.sub /^#{RESULT_VAR}_/, ''}"
        case dimension
        when 0
          out_var = "#{cast}(#{out_var})" if !cast.empty?
          code_str_list << "*#{name} = #{out_var};"
        when 1
          type_in     = cast.empty? ? type : cast
          type_out    = type
          name_array  = name
          name_vector = out_var
          elem        = "#{name_vector}.at(i)"
          if DEBUG
            code_str_list << "fmt::print(\"[C] #{name_array} = [{}]\\n\", fmt::join(#{name_vector}, \", \"));"
          end
          code_str_list += [
            "*#{name_array}__size = #{name_vector}.size();",
            "std::move(#{name_vector}.begin(), #{name_vector}.end(), #{name_array});",
          ]
        end
        code_str_list.join("\n  ")
      end
    end

    # -----------------------------------------

    # generate parts lists
    docstring_param_list = ['[in] algo_idx the algorithm index'] # list of doxygen docstrings
    par_list             = ['algo_idx_t* algo_idx'] # list of C function parameters
    arr2vec_list         = [] # list of converters from C array to C++ std::vector
    call_arg_list        = [] # list of C++ action function arguments
    assignment_list      = [] # list of return-value assignments
    case @ftn_type
    when 'filter'
      docstring_param_list += gen_docstring_params 'inputs', 'in'
      docstring_param_list << "[in,out] #{RESULT_VAR} the filter return value; if this value is already set, the result will be the `AND` of the initial value and this filter"
      docstring_param_list << "[in] #{RESULT_VAR}__size the size of `#{RESULT_VAR}`" if @ftn_rank == 'vector'
      par_list += gen_ftn_params 'inputs'
      par_list << "bool* #{RESULT_VAR}"
      par_list << "int* #{RESULT_VAR}__size" if @ftn_rank == 'vector'
      arr2vec_list += gen_arr2vec 'inputs'
      call_arg_list += gen_call_args 'inputs'
      case @ftn_rank
      when 'scalar'
        assignment_list << "*#{RESULT_VAR} = *#{RESULT_VAR} && out;"
      when 'vector'
        assignment_list << "*#{RESULT_VAR}__size = out.size();"
        assignment_list << "for(std::size_t i = 0; i < out.size(); i++)"
        assignment_list << "  #{RESULT_VAR}[i] = #{RESULT_VAR}[i] && out[i];"
      end
    when 'creator', 'transformer'
      docstring_param_list += gen_docstring_params 'inputs', 'in'
      docstring_param_list += gen_docstring_params 'outputs', 'out'
      par_list += gen_ftn_params 'inputs'
      par_list += gen_ftn_params 'outputs'
      arr2vec_list += gen_arr2vec 'inputs'
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
      /// - `iguana::#{@algo_name}::#{@ftn_name}`
      #{docstring_param_list.map{|s|"/// @param #{s}"}.join "\n"}
      void #{ftn_name_c}(
        #{par_list.join ",\n  "})
      {
        #{DEBUG ? "fmt::print(\"[C] CALL #{ftn_name_c}\\n\");" : ''}
        #{arr2vec_list.empty? ? '' : '// convert arrays to vectors'}
        #{arr2vec_list.join "\n  "}

        // call action function
        auto out = dynamic_cast<iguana::#{@algo_name}*>(iguana_get_algo_(algo_idx))->#{@ftn_name}(
          #{call_arg_list.join ",\n    "});

        // handle output
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
