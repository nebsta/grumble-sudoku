from conan import ConanFile

class Generator(ConanFile):
  settings = "os", "build_type", "arch"
  
  requires = ["glm/cci.20230113", "nlohmann_json/3.11.2", "grumble/1.0.0"]
    
  generators = "XcodeDeps"
