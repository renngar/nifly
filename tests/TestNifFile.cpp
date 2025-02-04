#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <NifFile.hpp>

using namespace nifly;

// WIP: Implement some proper testing

TEST_CASE("Load not existing file", "[NifFile]") {
	NifFile nif;
	REQUIRE(nif.Load("not_existing.nif") != 0);
}

TEST_CASE("Load and save static file (SE)", "[NifFile]") {
	NifFile nif;
	REQUIRE(nif.Load("TestNifFile_Static_SE.nif") == 0);
	REQUIRE(nif.Save("TestNifFile_Static_SE_output.nif") == 0);
}

TEST_CASE("Load and save static file (FO4)", "[NifFile]") {
	NifFile nif;
	REQUIRE(nif.Load("TestNifFile_Static_FO4.nif") == 0);
	REQUIRE(nif.Save("TestNifFile_Static_FO4_output.nif") == 0);
}

TEST_CASE("Load and save skinned file (SE)", "[NifFile]") {
	NifFile nif;
	REQUIRE(nif.Load("TestNifFile_Skinned_SE.nif") == 0);
	REQUIRE(nif.Save("TestNifFile_Skinned_SE_output.nif") == 0);
}

TEST_CASE("Load and save skinned file (FO4)", "[NifFile]") {
	NifFile nif;
	REQUIRE(nif.Load("TestNifFile_Skinned_FO4.nif") == 0);
	REQUIRE(nif.Save("TestNifFile_Skinned_FO4_output.nif") == 0);
}

TEST_CASE("Load and save animated file", "[NifFile]") {
	NifFile nif;
	REQUIRE(nif.Load("TestNifFile_Animated.nif") == 0);
	REQUIRE(nif.Save("TestNifFile_Animated_output.nif") == 0);
}

TEST_CASE("Load, optimize (LE to SE) and save file", "[NifFile]") {
	OptOptions options;
	options.targetVersion = NiVersion::getSSE();

	NifFile nif;
	REQUIRE(nif.Load("TestNifFile_Optimize_LE_to_SE.nif") == 0);
	nif.OptimizeFor(options);
	REQUIRE(nif.Save("TestNifFile_Optimize_LE_to_SE_output.nif") == 0);
}

TEST_CASE("Load, optimize (SE to LE) and save file", "[NifFile]") {
	OptOptions options;
	options.targetVersion = NiVersion::getSK();

	NifFile nif;
	REQUIRE(nif.Load("TestNifFile_Optimize_SE_to_LE.nif") == 0);
	nif.OptimizeFor(options);
	REQUIRE(nif.Save("TestNifFile_Optimize_SE_to_LE_output.nif") == 0);
}
