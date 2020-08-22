#include "ScriptCSharp.hpp"

#include <mono/jit/jit.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/assembly.h>

using namespace Grindstone;

bool ScriptCSharp::initialize() {
    mono_set_dirs("C:\\Program Files\\Mono\\lib", "C:\\Program Files\\Mono\\etc");

	domain_ = mono_jit_init_version("grindstone_mono_domain", "v4.0.30319");
	auto i = loadAssembly("scriptbin/Main.dll");

}

unsigned int ScriptCSharp::loadAssembly(std::string path) {
	MonoAssembly *assembly = mono_domain_assembly_open(domain_, path.c_str());
	MonoImage *image = mono_assembly_get_image(assembly);

	assemblies_.emplace_back(new AssemblyPackage(assembly, image, path));

	return (unsigned int)assemblies_.size() - 1;
}
