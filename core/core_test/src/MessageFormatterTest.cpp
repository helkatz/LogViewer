#define BOOST_SPIRIT_USE_PHOENIX_V3

#include <gui/logview/MessageFormatter.h>
#include <gtest/gtest.h>
#include <boost/any.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp> 
#include <boost/spirit/include/support_line_pos_iterator.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <map>
#include <iostream>
namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
namespace manip { struct LocationInfoPrinter; }
struct LocationInfo {
	unsigned line, column, length;
	manip::LocationInfoPrinter printLoc() const;
};

namespace manip {
	struct LocationInfoPrinter {
		LocationInfoPrinter(LocationInfo const& ref) : ref(ref) {}
		LocationInfo const& ref;
		friend std::ostream& operator<<(std::ostream& os, LocationInfoPrinter const& lip) {
			return os << lip.ref.line << ':' << lip.ref.column << ':' << lip.ref.length;
		}
	};
}

template<typename It>
struct annotation_f {
	typedef void result_type;

	annotation_f(It first) : first(first) {}
	It const first;

	template<typename Val, typename First, typename Last>
	void operator()(Val& v, First f, Last l) const {
		do_annotate(v, f, l, first);
	}
private:
	void static do_annotate(LocationInfo& li, It f, It l, It first) {
		using std::distance;
		li.line = get_line(f);
		li.column = get_column(first, f);
		li.length = distance(f, l);
	}
	static void do_annotate(...) {}
};
namespace parser_json {

	namespace qi = boost::spirit::qi;
	namespace ascii = boost::spirit::ascii;

	struct nullptr_t_ : qi::symbols< char, void* > {
		nullptr_t_() {
			add("null", nullptr);
		}
	} nullptr_;

	void print(boost::arg<1> arg1)
	{
		//std::cout << i << std::endl;
	}
	template< typename Iterator >
	struct Grammar : qi::grammar< Iterator, boost::any(), ascii::space_type > {
		Grammar() : 
			Grammar::base_type(start)
			//, annotate(start) 
		{
			using qi::lexeme;
			using qi::double_;
			using qi::bool_;
			using ascii::char_;

			start = value_rule.alias();
			object_rule = '{' >> pair_rule[boost::bind(&print, _1)] % ',' >> '}';
			pair_rule = string_rule >> ':' >> value_rule;
			value_rule = object_rule | array_rule | string_rule | nullptr_ | double_ | bool_;
			array_rule = '[' >> value_rule % ',' >> ']';
			string_rule = lexeme['\"' >> *(char_ - '\"') >> '\"'];

			//auto set_location_info = annotate(_val, _1, _3);
			//on_success(Identifier, set_location_info);
			//on_success(VarAssignment, set_location_info);
			//on_success(SourceCode, set_location_info);
		}

		qi::rule< Iterator, boost::any(), ascii::space_type > start;
		qi::rule< Iterator, std::map< std::string, boost::any >(), ascii::space_type > object_rule;
		qi::rule< Iterator, std::pair< std::string, boost::any >(), ascii::space_type > pair_rule;
		qi::rule< Iterator, boost::any(), ascii::space_type > value_rule;
		qi::rule< Iterator, std::vector< boost::any >(), ascii::space_type > array_rule;
		qi::rule< Iterator, std::string(), ascii::space_type > string_rule;

		//phx::function<annotation_f<Iterator>> annotate;
	};

}

TEST(MessageFormatterTest, boost_parse_json) {
	const std::string source = R"({"item1": true,"item2": "string"})";
	//const std::string source = "[ null ]";
	parser_json::Grammar< std::string::const_iterator > g;
	boost::any v;
	auto bit = source.begin();
	auto eit = source.end();
	bool r = boost::spirit::qi::phrase_parse(bit, eit, g, boost::spirit::ascii::space, v);
	try {
		if (r) {
			auto a = boost::any_cast<std::map<std::string, boost::any >>(v);
			//auto a = boost::any_cast<std::vector< boost::any >>(v);
			for (auto it = a.begin(); it != a.end(); ++it) {
				//std::cout << boost::any_cast<void*>(it->second) << std::endl;
			}
		}
	}
	catch (std::exception & e) {
		std::cout << e.what();
	}
}

TEST(MessageFormatterTest, format_json)
{

}