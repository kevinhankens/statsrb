require 'statsrb/statsrb'

# @author Kevin Hankens
class Statsrb
  # [!{:ts => Time.now.to_i, :ns => "test", :v => 33}]
  attr_accessor :data

  # Writes the @data in memory to a specified file.
  # @param filepath [String]
  # @param filemode [String]
  # @return Statsrb
  def write filepath, filemode
  end

  # Splits namespaces in @data in memory to a separate files.
  # @param filepath [String]
  # @param filemode [String]
  # @return Statsrb
  def split_write filepath, filemode
  end

  # Locates data from a specified file and loads into @data.
  # @param filepath [String]
  # @param namespace [String]
  # @param limit [Number]
  # @param start_time [Number]
  # @param end_time [Number]
  # @return Statsrb
  def query filepath, namespace, limit, start_time, end_time
  end

  # Returns a rack-compatable response.
  # @param env [Hash]
  def sort env
  end

end
