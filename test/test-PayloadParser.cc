//
//  test-PayloadParser.cc
//  snowcrash
//
//  Created by Zdenek Nemec on 5/7/13.
//  Copyright (c) 2013 Apiary.io. All rights reserved.
//

#include <iterator>
#include "catch.hpp"
#include "PayloadParser.h"

using namespace snowcrash;

//TEST_CASE("pldparser/classifier", "Asset block classifier")
//{
//    SourceData source = CanonicalBodyAssetSourceDataFixture;
//    MarkdownBlock::Stack markdown = CanonicalBodyAssetFixture();
//    
//    CHECK(markdown.size() == 6);
//    
//    BlockIterator cur = markdown.begin();
//    // ListBlockBeginType
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == BodySection);
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), BodySection) == BodySection);
//    
//    ++cur; // ListItemBlockBeginType
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == BodySection);
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), BodySection) == BodySection);
//    
//    ++cur; // ParagraphBlockType
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), BodySection) == BodySection);
//    
//    ++cur; // CodeBlockType
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), BodySection) == BodySection);
//    
//    ++cur; // ListItemBlockEndType
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), BodySection) == UndefinedSection);
//    
//    ++cur; // ListBlockEndType
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), UndefinedSection) == UndefinedSection);
//    REQUIRE(TClassifyBlock<Asset>(cur, markdown.end(), BodySection) == UndefinedSection);
//}

TEST_CASE("pldparser/parse-request", "Parse request payload")
{
    // Blueprint in question:
    //R"(
    //+ Request Hello World (text/plain)
    //
    //  Description
    //    + Body
    //
    //            Code
    //
    //)";

    SourceData source = "0123456789";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request Hello World (text/plain)", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Description", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Body", 0, MakeSourceDataBlock(4, 1)));
    markdown.push_back(MarkdownBlock(CodeBlockType, "Code", 0, MakeSourceDataBlock(5, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(6, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(7, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(8, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(9, 1)));
    
    Payload payload;
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 12);

    REQUIRE(payload.name == "Hello World");
    REQUIRE(payload.description == "1");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body == "Code");
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-incomplete", "Parse incomplete payload")
{
    // Blueprint in question:
    //R"(
    //+ Request A
    //
    //  Description
    //)";
    
    SourceData source = "01";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request A\n  B\n", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(1, 1)));
    
    Payload payload;
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    REQUIRE(result.first.warnings.empty());

    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 4);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description == "B");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-list-description", "Parse description with list")
{
    // Blueprint in question:
    //R"(
    //+ Request
    //  + B
    //  + Body
    //");
    
    SourceData source = "01234";
    MarkdownBlock::Stack markdown;

    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ParagraphBlockType, "Request", 0, MakeSourceDataBlock(0, 1)));
    
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "B", 0, MakeSourceDataBlock(1, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Body", 0, MakeSourceDataBlock(2, 1)));
    
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, SourceData(), 0, MakeSourceDataBlock(3, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(4, 1)));
    
    Payload payload;
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.size() == 1); // warn skipping body list
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 10);
    
    REQUIRE(payload.name.empty());
    REQUIRE(payload.description == "3");
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}

TEST_CASE("pldparser/parse-alien-listitem", "Parse alien list item after recognized one")
{
    // Blueprint in question:
    //R"(
    //+ Request A
    //+ Alien
    //)";
    
    SourceData source = "012";
    MarkdownBlock::Stack markdown;
    markdown.push_back(MarkdownBlock(ListBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock()));
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Request A", 0, MakeSourceDataBlock(0, 1)));
    markdown.push_back(MarkdownBlock(ListItemBlockBeginType, SourceData(), 0, SourceDataBlock())); 
    markdown.push_back(MarkdownBlock(ListItemBlockEndType, "Alien", 0, MakeSourceDataBlock(1, 1)));
    markdown.push_back(MarkdownBlock(ListBlockEndType, SourceData(), 0, MakeSourceDataBlock(2, 1)));
    
    Payload payload;
    ParseSectionResult result = PayloadParser::Parse(markdown.begin(), markdown.end(), source, Blueprint(), payload);
    
    REQUIRE(result.first.error.code == Error::OK);
    CHECK(result.first.warnings.empty());
    
    const MarkdownBlock::Stack &blocks = markdown;
    REQUIRE(std::distance(blocks.begin(), result.second) == 3);
    
    REQUIRE(payload.name == "A");
    REQUIRE(payload.description.empty());
    REQUIRE(payload.parameters.empty());
    REQUIRE(payload.headers.empty());
    REQUIRE(payload.body.empty());
    REQUIRE(payload.schema.empty());
}
