Gem::Specification.new do |s|
  s.name        = 'statsr'
  s.version     = '0.0.0'
  s.date        = '2012-02-24'
  s.summary     = "Statsr"
  s.description = "A ruby stats repository."
  s.authors     = ["Kevin Hankens"]
  s.email       = 'email@kevinhankens.com'
  s.files       = ["lib/statsr.rb"]
  s.homepage    =
    'http://rubygems.org/gems/statsr'

  s.files = Dir.glob('lib/**/*.rb') +
            Dir.glob('ext/**/*.{c,h,rb}')
  s.extensions = ['ext/statsr/extconf.rb']
end
