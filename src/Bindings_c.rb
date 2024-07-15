require './src/Chameleon.rb'

class Bindings_c < Chameleon

  ##################################################################################

  def initialize(out_name='', algo_name='')
    super(out_name, algo_name, 'C bindings')
    @out.puts <<~END_CODE
      // C functions to provide action function bindings for Fortran

      #{deterrence_banner 'c'}

      #include "../Algorithm.h"
      #include "iguana/algorithms/Bindings.h"

      // clang-format off
      namespace iguana::bindings::#{@algo_name} {
        extern "C" {
    END_CODE
  end

  ##################################################################################

  def bind(spec)
    ftn_name = get_spec spec, 'name'
    ftn_type = get_spec spec, 'type'
    verbose " - bind #{ftn_type} function #{@algo_name}::#{ftn_name}"
    check_function_type "#{@algo_name}::#{ftn_name}", ftn_type

    ### docstring
    @out.puts <<~END_CODE.gsub(/^/,'  ')

      /// @see `iguana::#{@algo_name}::#{ftn_name}`
      /// @param [in] algo_idx the algorithm index
    END_CODE
    get_spec(spec, 'inputs').each do |input|
      @out.puts "  /// @param [in] #{input}"
    end
    case ftn_type
    when 'filter'
      @out.puts <<~END_CODE.gsub(/^/,'  ')
        /// @param [in,out] out the filter return value; if this value is already set, the
        /// result will be the `AND` of the initial value and this filter
      END_CODE
    end

    ### function parameters
    @out.puts <<~END_CODE.gsub(/^/,'  ')
      void iguana_#{@algo_name.downcase.gsub /::/, '_'}_#{ftn_name.downcase}_(
        algo_idx_t* algo_idx,
    END_CODE
    get_spec(spec, 'inputs').each do |input|
      @out.puts "    #{get_spec input, 'type'}* #{get_spec input, 'name'},"
    end
    case ftn_type
    when 'filter'
      @out.puts "    bool* out)"
    end

    ### function body
    @out.puts "  {"
    case ftn_type
    when 'filter'
      @out.puts <<~END_CODE.gsub(/^/,'    ')
        *out = *out && dynamic_cast<iguana::#{@algo_name}*>(iguana_get_algo_(algo_idx))->#{ftn_name}(
          #{get_spec(spec, 'inputs').map{|i|"*#{get_spec i, 'name'}"}.join ', '});
      END_CODE
    end
    @out.puts "  }"

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
