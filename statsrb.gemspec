Gem::Specification.new do |s|
  s.name        = 'statsrb'
  s.version     = '0.1.2'
  s.date        = '2013-06-08'
  s.summary     = "Statsrb"
  s.description = "A ruby stats repository."
  s.authors     = ["Kevin Hankens"]
  s.email       = 'email@kevinhankens.com'
  s.files       = ["lib/statsrb.rb"]
  s.homepage    =
    'https://github.com/kevinhankens/statsrb'

  s.files = Dir.glob('lib/**/*.rb') +
            Dir.glob('ext/**/*.{c,h,rb}')
  s.extensions = ['ext/statsrb/extconf.rb']
end
