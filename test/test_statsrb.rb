require 'minitest/autorun'
require 'statsrb'
require 'json'
require 'pp'

class TestStatsrb < MiniTest::Test

  attr_accessor :tmpfile

  def setup
    @s = Statsrb.new
    @tmpfile = "/tmp/test.statsrb"
    @s.split_file_dir = "/tmp/"
    @s.flush_count = 10
  end

  def teardown
    File.delete @tmpfile unless !File.exists? @tmpfile
    rackfile = "/tmp/test"
    File.delete rackfile unless !File.exists? rackfile
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
    assert_equal @s.length, get_data.length
  end

  # Tests that we can filter the in-memory data.
  def test_get_data
    push_data
    t = @s.get "test1", 100, 0, 0
    assert_equal(t.length, 3);
    t = @s.get "test2", 100, 0, 0
    assert_equal(t.length, 2);
  end

  # Tests that we can sort the data.
  def test_sort_data
    current = 0
    push_data
    @s.sort
    t = @s.get "test1", 100, 0, 0
    t.each do |value|
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

  # Tests that we can clear data from memory.
  def test_clear_data
    push_data
    assert_equal @s.length, get_data.length
    @s.clear
    assert_equal @s.length, 0
  end

  # Tests that we can read data from a file.
  def test_read_data
    push_data
    write_data
    @s.clear
    @s.read @tmpfile, "test1", 100, 0, 0
    assert_equal @s.length, 3
    @s.clear
    @s.read @tmpfile, "test2", 100, 0, 0
    assert_equal @s.length, 2
  end

  # Tests that the rack interface works properly.
  def test_rack_call
    # Test putting data.
    env = {
      "PATH_INFO" => "/PUT",
      "QUERY_STRING" => "name=test&value=13"
    }

    5.times do |i|
      @s.call(env);
    end

    assert_equal 5, @s.length

    # Write enough data to flush.
    5.times do |i|
      @s.call(env);
    end

    # Test getting data.
    env = {
      "PATH_INFO" => "/GET/test",
      "QUERY_STRING" => ""
    }

    resp = @s.call(env)
    data = JSON.parse(resp[2].join)
    assert_equal data["test"].length, 10
  end

  def test_no_results
    push_data
    t = @s.get "noresults", 100, 0, 0
    assert_equal(t.length, 0);
  end

  # Tests large data volumes.
  def test_large_data
    # Load a lot of data.
    @s.load_test "kevin", 500000
    @s.load_test "melissa", 500000
    @s.load_test "benjamin", 500000
    @s.sort
    # Extract all of one namespace.
    t = @s.get "melissa", 100000, 0, 0
    assert_equal t.length, 100000
    # Push them back to the object.
    t.each do |i|
      @s.push i[:ts], i[:ns], i[:v]
    end
    # Save it to file and clear it.
    @s.write @tmpfile, "w+"
    @s.clear
    # Re-load the data.
    @s.read @tmpfile, "melissa", 600000, 0, 0
    # Try to get one that doesn't exist.
    t = @s.get "kevin", 10000, 0, 0
    assert_equal t.length, 0
    # Try to get all of the data out.
    t = @s.get "melissa", 600000, 0, 0
    assert_equal t.length, 600000
  end
end
