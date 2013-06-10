require 'minitest/autorun'
require 'statsrb'

class TestStatsrb < MiniTest::Test

  attr_accessor :tmpfile

  def setup
    @s = Statsrb.new
    @tmpfile = "/tmp/test.statsrb"
  end

  def teardown
    File.delete @tmpfile unless !File.exists? @tmpfile
  end

  # Provides test data.
  def get_data
    [[123, "test1", 33],
    [321, "test1", 34],
    [222, "test1", 35],
    [111, "test2", 36],
    [432, "test2", 37]]
  end

  # Stores the data in the current object.
  def push_data
    get_data.each do |value|
      @s.push value[0], value[1], value[2]
    end
  end

  # Writes the data to a temp file.
  def write_data
    @s.write @tmpfile, "w+"
  end

  # Tests that the data was indeed pushed.
  def test_push_data
    push_data
    assert_equal @s.data.length, get_data.length
  end
  
  # Tests that we can filter the in-memory data.
  def test_get_data
    push_data
    t = @s.get "test2", 100, 0, 0
    assert_equal(t.length, 2);
  end

  # Tests that we can sort the data.
  def test_sort_data
    current = 0
    push_data
    @s.sort
    @s.data.each do |value|
      assert value[:ts] > current
      current = value[:ts]
    end
  end

  # Tests that we can write the data to a file.
  def test_write_data
    push_data
    write_data
    data = get_data
    file_data = File.read(@tmpfile).split "\n"

    assert_equal file_data.length, data.length
    count = 0
    file_data.each do |value|
      parts = value.split "\t"
      assert_equal parts[0], data[count][0].to_s
      assert_equal parts[1], data[count][1].to_s
      assert_equal parts[2], data[count][2].to_s
      count = count + 1
    end
  end

  # Tests that we can read data from a file.
  def test_read_data
    push_data
    write_data
    @s.read @tmpfile, "test1", 100, 0, 0
    assert_equal @s.data.length, 3
    @s.read @tmpfile, "test2", 100, 0, 0
    assert_equal @s.data.length, 2
  end
end
