require "option_parser"
require "yaml"

OptionParser.parse! do |opt|
  opt.banner = "Usage: hog [arguments]"
  opt.on "-v", "--version", "Show version number" do
    shard = YAML.load(File.read "shard.yml") as Hash
    puts "#{shard["name"]}-#{shard["version"]}"
    exit
  end
  opt.on "-h", "--help", "Show this help" {puts opt; exit}
end
puts 12
