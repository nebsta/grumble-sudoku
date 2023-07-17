from conan import ConanFile

class Generator(ConanFile):
  settings = "os", "build_type", "arch"
  
  requires = ["grumble/1.0.0"]
    
  generators = "XcodeDeps"
