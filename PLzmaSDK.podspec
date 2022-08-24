Pod::Spec.new do |s|
  s.name = 'PLzmaSDK'
  s.version = '1.2.2'
  s.summary = '(Portable, Patched, Package, cross-P-latform) Lzma SDK, libplzma'
  s.homepage = 'https://github.com/OlehKulykov/PLzmaSDK'
  s.source = { :git => s.homepage + '.git', :tag => s.version }
  s.license = { :type => 'MIT', :file => 'LICENSE' }
  s.authors = { 'Oleh Kulykov' => 'olehkulykov@gmail.com' }
  s.swift_versions = ['5.0', '5.1', '5.2', '5.3', '5.4', '5.5', '5.6']
  s.ios.deployment_target = '9.0'
  s.osx.deployment_target = '10.9'
  s.tvos.deployment_target = '9.0'
  s.watchos.deployment_target = '2.0'
  
  s.subspec 'libplzma' do |libplzma|
    libplzma.source_files = 'libplzma.h', 'libplzma.hpp', 'src/**/*.{c,cpp,h,hpp}'
    libplzma.public_header_files = 'libplzma.h'
    libplzma.compiler_flags = '-DLIBPLZMA_VERSION_BUILD=468'
    libplzma.libraries = 'c++'
  end
  
  s.subspec 'Swift' do |swift|
    swift.source_files = 'swift/*.swift'
    swift.dependency 'PLzmaSDK/libplzma'
  end
  
end
