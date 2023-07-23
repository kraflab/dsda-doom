# Converts from zm's format to spec format

filename = "raven_demos.txt"

lines = File.readlines(filename, chomp: true).map { |x| x.split('|') }

lines.shift # header

lines.each do |line|
  if line.size != 16
    puts "bad line %{line}"
    raise "invalid line"
  end
end

lines.map! do |line|
  time = line[5].split('.')[0]
  components = time.split(':')
  if components.length > 2
    h = components.shift.to_i
    components[0] = (h * 60 + components[0].to_i).to_s
  end
  time = components.join(':')

  params = line[14].split(' ').keep_if do |x|
    x == '-respawn' || x == '-nomonsters'
  end.join(' ')

  players = line[6].split('#').join(', ')

  {
    iwad: line[1],
    wad: line[2],
    map: line[3],
    category: line[4],
    time: time,
    players: players,
    number: line[9],
    lmp: line[11],
    params: params
  }
end

lines.keep_if { |x| x[:wad] == 'heretic' }

lines.each do |demo|
  puts "#{demo[:wad]} #{demo[:map]} #{demo[:category]} in #{demo[:time]} by #{demo[:players]}|#{demo[:time]}|#{demo[:wad]}/#{demo[:number]}/#{demo[:lmp]}|#{demo[:params]}"
end
