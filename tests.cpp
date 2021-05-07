#include <gtest/gtest.h>
#include "config/config.h"
#include "database/database.h"
#include "database/person.h"
#include <Poco/Data/SessionFactory.h>
using Poco::Data::Session;
using Poco::Data::Statement;
using namespace Poco::Data::Keywords;

long get_ai(int shard_num) {

    int result;

    Poco::Data::Session session = database::Database::get().create_session();
    Statement get_ai(session);
    std::string sql_request = "Select count(*) from Person -- sharding:" + std::to_string(shard_num);
    get_ai << sql_request, into(result), now;

    return result;
}

void reset_ai(int shard_num) {
    Poco::Data::Session session = database::Database::get().create_session();
    Statement reset_ai(session);
    std::string sql_request = "ALTER TABLE Person AUTO_INCREMENT = " + std::to_string((shard_num) + 1) + " -- sharding:" + std::to_string(shard_num);
    reset_ai << sql_request, now;
}

void remove_person(std::string login, int shard_num) {
    Poco::Data::Session session = database::Database::get().create_session();
    Statement cleanup(session);
    std::string sql_request = "DELETE FROM Person where login = ? -- sharding:" + std::to_string(shard_num);
    cleanup << sql_request, use(login), now;
}

class TestApp : public ::testing::Test {
protected:
    TestApp() {
        Config::get().host() = "127.0.0.1";
        Config::get().database() = "sql_test";
        Config::get().port() = "6033";
        Config::get().login() = "test";
        Config::get().password() = "pzjqUkMnc7vfNHET";
    }
    
    ~TestApp() {
        remove_person("111", 0);
        remove_person("222", 1);
        remove_person("333-333-333", 1);
        remove_person("54321", 2);
        remove_person("12345", 0);

        reset_ai(0);
        reset_ai(1);
        reset_ai(2);
    }
    
    void SetUp() {}
    
    void TearDown() {}

protected:

};

TEST_F(TestApp, TestPerson) {

    database::Person person;
    /*
    std::cout << "Shard #0: " << get_ai(0) << std::endl;
    std::cout << "Shard #1: " << get_ai(1) << std::endl;
    std::cout << "Shard #2: " << get_ai(2) << std::endl;
    */
    //POST tests
    person.login() = "111";
    person.first_name() = "Anton";
    person.last_name() = "Larin";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "-- sharding:0\n");

    person.login() = "222";
    person.first_name() = "A";
    person.last_name() = "L";
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "-- sharding:1\n");

    person.login() = "333-333-333";
    person.first_name() = "Alexey";
    person.last_name() = "Vorobev";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "-- sharding:1\n");

    person.login() = "54321";
    person.first_name() = "Vlad";
    person.last_name() = "Petrushin";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "-- sharding:2\n");

    person.login() = "12345";
    person.first_name() = "Alexey";
    person.last_name() = "Vinnikov";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "-- sharding:0\n");

    //GET tests
    database::Person login_result1 = database::Person::read_by_login("111");
    ASSERT_EQ(login_result1.get_first_name(), "Anton");
    ASSERT_EQ(login_result1.get_last_name(), "Larin");
    ASSERT_EQ(login_result1.get_age(), 22);

    database::Person login_result2 = database::Person::read_by_login("222");
    ASSERT_EQ(login_result2.get_first_name(), "A");
    ASSERT_EQ(login_result2.get_last_name(), "L");
    ASSERT_EQ(login_result2.get_age(), 22);

    auto name_result1 = database::Person::search("Anton", "Larin");
    ASSERT_EQ(name_result1.at(0).get_login(), "111");
    ASSERT_EQ(name_result1.size(), 1);

    auto name_result2 = database::Person::search("Alexey", "V");
    ASSERT_EQ(name_result2.size(), 2);

    auto full_query = database::Person::read_all();
    ASSERT_EQ(full_query.size(), 100005);
}


int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}