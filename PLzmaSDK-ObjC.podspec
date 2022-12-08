Pod::Spec.new do |s|
  s.name = 'PLzmaSDK-ObjC'
  s.version = '1.2.5'
  s.summary = '(Portable, Patched, Package, cross-P-latform) Lzma SDK, libplzma'
  s.homepage = 'https://github.com/OlehKulykov/PLzmaSDK'
  s.source = { :git => s.homepage + '.git', :tag => s.version }
  s.license = { :type => 'MIT', :file => 'LICENSE' }
  s.authors = { 'Oleh Kulykov' => 'olehkulykov@gmail.com' }
  s.ios.deployment_target = '9.0'
  s.osx.deployment_target = '10.9'
  s.tvos.deployment_target = '9.0'
  s.watchos.deployment_target = '2.0'
  
  s.source_files = 'libplzma.h', 'libplzma.hpp', 'src/**/*.{c,cpp,h,hpp}', 'objc/*.{mm,h,inl}'
  s.public_header_files = 'objc/*.h'
  s.compiler_flags = '-DLIBPLZMA_VERSION_BUILD=506', '-DLIBPLZMA_NO_C_BINDINGS=1'
  s.libraries = 'c++'
  s.requires_arc = true
  
end
