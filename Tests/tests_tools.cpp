#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "../Include/tools.h"

using namespace Catch;

TEST_CASE("Vec3 Constructor", "[Vec3]")
{
    Vec3 v(1.0f, 2.0f, 3.0f);
    REQUIRE(v.x() == Approx(1.0f));
    REQUIRE(v.y() == Approx(2.0f));
    REQUIRE(v.z() == Approx(3.0f));
}

TEST_CASE("Vec3 Addition", "[Vec3]")
{
    Vec3 v1(1.0f, 2.0f, 3.0f);
    Vec3 v2(4.0f, 5.0f, 6.0f);
    Vec3 result = v1 + v2;
    REQUIRE(result.x() == Approx(5.0f));
    REQUIRE(result.y() == Approx(7.0f));
    REQUIRE(result.z() == Approx(9.0f));
}

TEST_CASE("Vec3 Subtraction", "[Vec3]")
{
    Vec3 v1(4.0f, 5.0f, 6.0f);
    Vec3 v2(1.0f, 2.0f, 3.0f);
    Vec3 result = v1 - v2;
    REQUIRE(result.x() == Approx(3.0f));
    REQUIRE(result.y() == Approx(3.0f));
    REQUIRE(result.z() == Approx(3.0f));
}

TEST_CASE("Vec3 Scalar Multiplication", "[Vec3]")
{
    Vec3 v(1.0f, 2.0f, 3.0f);
    Vec3 result = v * 2.0f;
    REQUIRE(result.x() == Approx(2.0f));
    REQUIRE(result.y() == Approx(4.0f));
    REQUIRE(result.z() == Approx(6.0f));
}

TEST_CASE("Vec3 Dot Product", "[Vec3]")
{
    Vec3 v1(1.0f, 2.0f, 3.0f);
    Vec3 v2(4.0f, 5.0f, 6.0f);
    REQUIRE(v1.dot(v2) == Approx(32.0f));
}

TEST_CASE("Vec3 Cross Product", "[Vec3]")
{
    Vec3 v1(1.0f, 0.0f, 0.0f);
    Vec3 v2(0.0f, 1.0f, 0.0f);
    Vec3 result = v1.cross(v2);
    REQUIRE(result.x() == Approx(0.0f));
    REQUIRE(result.y() == Approx(0.0f));
    REQUIRE(result.z() == Approx(1.0f));
}

TEST_CASE("Vec3 Normalization", "[Vec3]")
{
    Vec3 v(3.0f, 4.0f, 0.0f);
    Vec3 norm = v.normalized();
    REQUIRE(norm.length() == Approx(1.0f));
    REQUIRE(norm.x() == Approx(3.0f / 5.0f));
    REQUIRE(norm.y() == Approx(4.0f / 5.0f));
    REQUIRE(norm.z() == Approx(0.0f));
}

TEST_CASE("Vec3 Length and Norm2", "[Vec3]")
{
    Vec3 v(3.0f, 4.0f, 0.0f);
    REQUIRE(v.length() == Approx(5.0f));
    REQUIRE(v.norm2() == Approx(25.0f));
}

TEST_CASE("Remove XML Comments", "[XML]")
{
    std::string xml = "<tag><!-- This is a comment -->content</tag>";
    REQUIRE(removeComments(xml) == "<tag>content</tag>");
}

TEST_CASE("Parse XML Bracket", "[XML]")
{
    std::string xml = "<node attr=\"value\"><child></child></node>";
    XML_Node node = parse_xml_bracket(xml);
    REQUIRE(node.tag_name == "node");
    REQUIRE(node.parameters["attr"] == "value");
    REQUIRE(node.children.size() == 1);
}