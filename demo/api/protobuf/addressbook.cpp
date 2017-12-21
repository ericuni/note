#include <stdio.h>
#include <string>
#include <gtest/gtest.h>
#include "addressbook.pb.h"

const int max_vl = 10 * 1024 * 1024;

using namespace std;
wcg::pb::ex::AddressBook address_book;

int add_person(wcg::pb::ex::AddressBook &address_book, const char *name, const char *email, const char *phone_number);
int print(wcg::pb::ex::AddressBook &address_book);

TEST(pbTest, Test_add_person){
	ASSERT_TRUE(add_person(address_book, "xxx", "xxx@xxx.com", "123456") == 0);
	ASSERT_TRUE(add_person(address_book, "yyy", "yyy@yyy.com", "654321") == 0);
	ASSERT_TRUE(address_book.person_size() == 2);
}

TEST(pbTest, Test_string){
	std::string v;
	ASSERT_TRUE(address_book.SerializeToString(&v) == true);

	wcg::pb::ex::AddressBook ex;
	ASSERT_TRUE(ex.ParseFromString(v) == true);
	print(ex);
}

TEST(pbTest, Test_char){
	char *v = new char[max_vl];
	int vl = address_book.ByteSize();
	ASSERT_TRUE(address_book.SerializeToArray(v, max_vl) == true);

	wcg::pb::ex::AddressBook ex;
	ASSERT_TRUE(ex.ParseFromArray(v, vl) == true);
	print(ex);
}

TEST(pbTest, Test_file){
	const char *path = "./data.pb";
	FILE *fp = fopen(path, "wb");
	int fd = fileno(fp);
	ASSERT_TRUE(fp != NULL);
	ASSERT_TRUE(fd > 0);

	ASSERT_TRUE(address_book.SerializeToFileDescriptor(fd) == true);
	fclose(fp);
	fp = NULL;
	fd = -1;

	fp = fopen(path, "rb");
	fd = fileno(fp);
	ASSERT_TRUE(fp != NULL);
	ASSERT_TRUE(fd > 0);

	wcg::pb::ex::AddressBook ex;
	ASSERT_TRUE(ex.ParseFromFileDescriptor(fd) == true);
	fclose(fp);
	fp = NULL;
	fd = -1;

	print(ex);
}

#include <fstream>
TEST(pbTest, Test_stream){
	const char *path = "./data.pb";
	fstream output(path, ios::out | ios::trunc | ios::binary);
	ASSERT_TRUE(address_book.SerializeToOstream(&output) == true);

	fstream input(path, ios::in | ios::binary);
	wcg::pb::ex::AddressBook ex;
	ASSERT_TRUE(ex.ParseFromIstream(&input) == true);
	print(ex);
}

int main(int argc, char* argv[]){
	// Verify that the version of the library that we linked against is compatible with the version of the headers we compiled against.
// 	GOOGLE_PROTOBUF_VERIFY_VERSION;

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();

	// Optional: Delete all global objects allocated by libprotobuf
// 	google::protobuf::ShutdownProtobufLibrary();

// 	return 0;
}

int add_person(wcg::pb::ex::AddressBook &address_book, const char *name, const char *email, const char *phone_number){
	int id = address_book.person_size();
	wcg::pb::ex::Person *person = address_book.add_person();
	person->set_name(name);
	person->set_email(email);

	wcg::pb::ex::PhoneNumber* phone = person->add_phone();
	phone->set_number(phone_number);
	switch(id % 3){
		case 0:
			phone->set_type(wcg::pb::ex::MOBILE);
			break;
		case 1:
			phone->set_type(wcg::pb::ex::HOME);
			break;
		case 2:
			phone->set_type(wcg::pb::ex::WORK);
			break;
	}

	return 0;
}

int print(wcg::pb::ex::AddressBook &address_book){
	for(int i = 0; i < address_book.person_size(); i++){
		const wcg::pb::ex::Person& person = address_book.person(i);

		fprintf(stdout, "Name: %s\t", person.name().c_str());
		if(person.has_email()){
			fprintf(stdout, "email: %s\t", person.email().c_str());
		}

		for(int j = 0; j < person.phone_size(); j++){
			const wcg::pb::ex::PhoneNumber& phone_number = person.phone(j);

			switch(phone_number.type()){
				case wcg::pb::ex::MOBILE:
					fprintf(stdout, "Mobile phone: ");
					break;
				case wcg::pb::ex::HOME:
					fprintf(stdout, "Home phone: ");
					break;
				case wcg::pb::ex::WORK:
					fprintf(stdout, "Work phone: ");
					break;
			}
			fprintf(stdout, "%s\n", phone_number.number().c_str());
		}
	}

	return 0;
}

