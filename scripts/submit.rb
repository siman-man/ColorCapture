require 'net/https'
require 'json'

class Submit
  TSV_LABEL = ['seed', 'score', 'time'].join("\t")

  def initialize(round_id, submit_type, filename)
    filepath = File.expand_path(filename, Dir::pwd)
    data_list = parse
    time_stamp = Time.now.strftime("%m%d-%H%m%S")

    uri = URI.parse('http://localhost:3000/rounds/submits')
    http = Net::HTTP.new(uri.host, uri.port)

    body = {
      round_id: round_id,
      submit_type: submit_type,
      code: File.read(filepath)
    }

    scores = {}

    data_list.each do |data|
      data = clean_data(data)

      seed = data[0]
      score = data[1]

      scores[seed] = score
    end

    body[:scores] = scores

    req = Net::HTTP::Post.new(uri.path)
    req.set_form_data(body)

    res = http.request(req)
  end 

  def clean_data(data)
    seed = data['seed'].to_i
    score = data['score'].to_f
    /(?<minute>\d+)m(?<second>(\d|\.)+)s/ =~ data['user']
    time = minute.to_f * 60 + second.to_f

    [seed, score, time]
  end

  def parse
    filepath = File.expand_path('result.txt', Dir::pwd)
    data_list = []
    data = {}

    File.open(filepath, 'r') do |file|
      file.each_line do |line|
        line = clean_line(line)

        if line =~ /begin/
          data = {}
        elsif line =~ /!end!/
          data_list << data.dup
        else
          if validate(line)
            h = line2hash(line)
            data.merge!(h)
          end
        end
      end
    end

    data_list
  end

  def line2hash(line)
    Hash[*line.split]
  end

  def clean_line(line)
    line.chomp.downcase.delete('=')
  end

  def validate(line)
    line =~ /^(score|seed|user)/
  end
end

round_id = ARGV[0]
submit_type = ARGV[1]
filepath = ARGV[2]

Submit.new(round_id, submit_type, filepath)
