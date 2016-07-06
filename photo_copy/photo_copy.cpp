
#include "stdafx.h"
#include <iostream>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/algorithm/find_if.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

#pragma warning(disable:4996)
#pragma comment(lib,"libboost_filesystem-vc140-mt-gd-1_61.lib")

std::wstring get_date_string(const boost::filesystem::path& infile) {
	boost::date_time::time_facet<boost::posix_time::ptime, wchar_t>* facet(
		new boost::date_time::time_facet<boost::posix_time::ptime, wchar_t>(L"%Y_%m_%d"));
	std::wostringstream	oss;
	oss.imbue(std::locale(std::wcout.getloc(), facet));
	boost::date_time::local_adjustor<boost::posix_time::ptime, 9, boost::posix_time::no_dst> adj;
	boost::posix_time::ptime file_time = boost::posix_time::from_time_t(boost::filesystem::last_write_time(infile));
	oss << adj.utc_to_local(file_time);
	return oss.str();
}


void copy_to_distination(std::wstring src, std::wstring dst, std::vector<std::wstring>& ext) {
	const boost::filesystem::path src_path(src);
	BOOST_FOREACH(	const boost::filesystem::path& src_file,
					std::make_pair(boost::filesystem::recursive_directory_iterator(src_path),
					boost::filesystem::recursive_directory_iterator())) {
		if (!boost::filesystem::is_directory(src_file)) {
			std::wstring file_extension = src_file.extension().wstring();
			boost::to_lower(file_extension);
			std::vector<std::wstring>::iterator it = boost::find_if(ext, boost::lambda::_1 == file_extension);
			if (it != ext.end()) {
				std::wstring directory = get_date_string(src_file);
				boost::filesystem::path dst_path(dst);
				dst_path = dst_path / directory;
				if (!boost::filesystem::exists(dst_path))
					boost::filesystem::create_directory(dst_path);
				std::wcout << L"copy from: " << src_file << L" to: " << dst_path / src_file.leaf() << std::endl;
				boost::filesystem::rename(src_file, dst_path / src_file.leaf());
				boost::filesystem::remove(src_file);
			}
		}
	}
}


int main( int argc, char* argv[] ){
	boost::program_options::options_description opt("options");
	opt.add_options()
		("help,h", "show help")
		("src,s", boost::program_options::wvalue<std::wstring>(), "source directory")
		("dst,d", boost::program_options::wvalue<std::wstring>(), "destination directory")
		("ext,e", boost::program_options::wvalue<std::wstring>(), "copy extend");
	boost::program_options::variables_map vm;
	try {
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, opt), vm);
	}
	catch (const boost::program_options::error_with_no_option_name& e) {
		std::cout << e.what() << std::endl;
		return -1;
	}

	boost::program_options::notify(vm);

	if (vm.count("help")) {
		std::cout << opt << std::endl;
	}
	else {
		if (!vm.count("src")) {
			std::cout << "please set -s,--src option" << std::endl;
			return -1;
		}
		if (!vm.count("dst")) {
			std::cout << "please set -d,--dst option" << std::endl;
			return -1;
		}
		const std::wstring src_dir = vm["src"].as<std::wstring>();
		const std::wstring dst_dir = vm["dst"].as<std::wstring>();
		std::vector<std::wstring> exts;
		if( vm.count("ext") ) boost::split(exts, vm["ext"].as<std::wstring>(), boost::is_any_of<std::wstring>(L","));
		exts.push_back(L".jpg");
		exts.push_back(L".jpeg");
		exts.push_back(L".rw2");
		exts.push_back(L".orf");
		copy_to_distination(src_dir, dst_dir, exts);
	}


    return 0;
}

