//#include <vector>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//
//void stringBasicUsage();
//void stringToNumberConversion();
//void stringStreamBasicUsage();
//void stringStreamToParseNumbers();
//void numberToStringExample();
//void getlineExample();
//void anotherVectorExample();
//
//int main()
//{
//    stringBasicUsage();
//    stringToNumberConversion();
//    stringStreamBasicUsage();
//    stringStreamToParseNumbers();
//    numberToStringExample();
//    getlineExample();
//    anotherVectorExample();
//}
//
//void stringBasicUsage()
//{
//    std::string s1 = "Hello world!";
//    auto s2 = std::string("Hello world!");
//
//    std::cout << s1[4] << '\n';
//    std::cout << s1.size() << '\n';
//
//    // Can be compared directly and it compares the content
//    std::cout << "s1 == s2: " << (s1 == s2) << '\n';
//
//    // Can be concatenated easily with + and += operators
//    auto s3 = s1 + " - " + s2;
//    s3 += '!';
//    std::cout << s3 << '\n';
//    std::cout << s3.size() << '\n';
//}
//
//void stringToNumberConversion()
//{
//    auto numberString = std::string("5");
//    int number = std::stoi(numberString);
//    std::cout << number << '\n';
//}
//
//void stringStreamBasicUsage()
//{
//    // Reading from a string like it's a stream (like std::cin)
//    // It reads words one-by-one
//    std::string s1 = "Hello world!";
//    auto iss = std::istringstream(s1);
//    std::string temp;
//    while (iss >> temp)
//    {
//        std::cout << temp << '\n';
//    }
//}
//
//void stringStreamToParseNumbers()
//{
//    // And similarly for numbers
//    auto issNumbers = std::istringstream("42 0 fail 7");
//    int num = 0;
//    while (issNumbers >> num)
//    {
//        std::cout << num << '\n';
//    }
//}
//
//void numberToStringExample()
//{
//    auto i = 42;
//    auto str = std::to_string(i);
//    auto quote = "The answer is: " + std::to_string(42) + "!!!";
//    std::cout << quote << '\n';
//}
//
//void getlineExample()
//{
//    // R"(...)" - raw string, a string where backslash (\) is used literally, not as escaping
//    // Very convenient for Windows-style file path
//    auto license = std::ifstream(R"(C:\Program Files (x86)\Notepad++\LICENSE)");
//
//    // Read a file line-by-line
//    auto licenseAsLines = std::vector<std::string>();
//    for (auto line = std::string(); std::getline(license, line); )
//    {
//        std::cout << line.size() << '\n';
//        licenseAsLines.push_back(line);
//    }
//
//    std::cout << "Number of rows in license file: " << licenseAsLines.size() << '\n';
//
//    // Dangerous if the line is empty (or no line was read from the file)!
//    std::cout << "The 2nd char in the 1st line is: " << licenseAsLines[0][1] << '\n';
//}
//
//void anotherVectorExample()
//{
//    // vector can hold any type we want
//    auto vi = std::vector<int>();
//    vi.push_back(7);
//
//    // Notice the difference between clear() and empty()!
//    std::cout << "The vector is" << (vi.empty() ? "" : " not") << " empty\n";
//    vi.clear();
//    std::cout << "The vector is" << (vi.empty() ? "" : " not") << " empty\n";
//}
