#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <algorithm>
#include <cstring>
#include <locale>
#include <string>
#include <qstring.h>
#include <vector>
#include <qdatetime.h>

#define STRSTR(buff, str) Utils::ustrstr(buff, str)

using namespace std;

template<typename charT>
struct my_equal {
    my_equal( const locale loc ) : loc_(loc) {}
    bool operator()(charT ch1, charT ch2) {
        return tolower(ch1, loc_) == tolower(ch2, loc_);
    }
private:
    const locale loc_;
};

class Utils {
private: static std::string startDate;
private: static std::string startTime;
private: static std::string currentTarget;
public:
	static int isDigest(const std::string *buffer);

    // find substring (case insensitive)
    template<typename T> static int ustrstr(const T& str1,
                                            const T& str2,
                                            const locale& loc = locale()) {

        auto it = std::search(str1.begin(), str1.end(), str2.begin(), str2.end(),
                              my_equal<typename T::value_type>(loc));
        if(it != str1.end()) return it - str1.begin();
        else return -1;
	}

	template<typename T> static int ustrstr(const T& str1,
		const char* str2c,
		const locale& loc = locale()) {

		std::string str2 = std::string(str2c);
		auto it = std::search(str1.begin(), str1.end(), str2.begin(), str2.end(),
			my_equal<typename T::value_type>(loc));
		if (it != str1.end()) return it - str1.begin();
		else return -1;
	}

	template<typename T> static int ustrstr(T *str1,
		const char* str2c,
		const locale& loc = locale()) {

		std::string str2 = std::string(str2c);
		auto it = std::search(str1->begin(), str1->end(), str2.begin(), str2.end(),
			my_equal<typename T::value_type>(loc));
		if (it != str1->end()) return it - str1->begin();
		else return -1;
	}

	static QString GetNSErrorDefinition(const char *str, const char *elem){
		const QString missing("No definition found!");
		if (str == nullptr || elem == nullptr || *elem == '\0') return missing;
		const char *temp = strstr(str, elem);
		if (temp == nullptr) return missing;
		const char *afterElement = temp + strlen(elem);
		if (*afterElement == '\0') return missing;
		const char *firstComma = strchr(afterElement + 1, '"');
		if (firstComma == nullptr) return missing;
		const char *lastComma = strchr(firstComma + 1, '"');
		if (lastComma == nullptr) return missing;
		const auto length = std::min<std::ptrdiff_t>(lastComma - firstComma - 1, 128);
		return QString::fromUtf8(firstComma + 1, static_cast<int>(length));
	}

    char * getProxy();
    int getProxyPort();
	static std::string getStrValue(const std::string &data, const std::string &delim1, const std::string &delim2);
	static std::vector<std::string> splitToStrVector(const std::string &s, char delim);
	static std::vector<int> splitToIntVector(const std::string &s, char delim);
	static void saveStartDate();
	static void saveStartTime();
	static std::string getStartDate();
	static std::string getStartTime();
	static void setCurrentTarget(const std::string target);
	static std::string getCurrentTarget();
	static void emitScaryError();
	static int addBARow(QString str1, QString str2, QString str3, int rowIndex);
	static std::string getHeaderValue(std::string *buff, const std::string headerValue, const std::string outputName);
};

#endif // UTILS_H

