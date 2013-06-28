Gem::Specification.new do |s|
  s.name        = 'statsrb'
  s.version     = '0.2.0'
  s.date        = '2013-06-28'
  s.summary     = "A Ruby time series stats repository."
  s.description = "A Ruby time series stats repository written in C, using flat file storage, providing a Ruby API as well as a Rack compatible REST API."
  s.authors     = ["Kevin Hankens"]
  s.email       = 'email@kevinhankens.com'
  s.files       = ["lib/statsrb.rb"]
  s.homepage    =
    'https://github.com/kevinhankens/statsrb'
  s.post_install_message = 'Please view documentation at: ' + s.homepage
  s.license = 'MIT	'
  s.files = Dir.glob('lib/**/*.rb') +
            Dir.glob('ext/**/*.{c,h,rb}')
  s.extensions = ['ext/statsrb/extconf.rb']
  s.test_files = Dir.glob('test/*.rb')
end
