#include <gtest/gtest.h>
#include "../config/config.h"
#include "../database/database.h"
#include "../database/person.h"
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
public:
    TestApp() {
        Config::get().host() = "127.0.0.1";
        Config::get().database() = "sql_test";
        Config::get().port() = "6033";
        Config::get().login() = "test";
        Config::get().password() = "pzjqUkMnc7vfNHET";
        Config::get().cache_servers() = "127.0.0.1:10800,127.0.0.1:10900";
        Config::get().queue_host() = "127.0.0.1:9092";
        Config::get().queue_topic() = "event_server";
        Config::get().queue_group_id() = "0";
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

TEST_F (TestApp, test1) {
    database::Person person;
    person.login() = "111";
    person.first_name() = "Anton";
    person.last_name() = "Larin";
    person.age() = 22;
    person.save_to_mysql();
    database::Person login_result = database::Person::read_by_login(person.get_login());
    ASSERT_EQ(login_result.get_first_name(), person.get_first_name());
    ASSERT_EQ(login_result.get_last_name(), person.get_last_name());
    ASSERT_EQ(login_result.get_age(), person.get_age());
}

TEST_F (TestApp, test2) {
    database::Person person;
    person.login() = "222";
    person.first_name() = "A";
    person.last_name() = "L";
    person.age() = 22;
    person.save_to_mysql();
    database::Person login_result = database::Person::read_by_login(person.get_login());
    ASSERT_EQ(login_result.get_first_name(), person.get_first_name());
    ASSERT_EQ(login_result.get_last_name(), person.get_last_name());
    ASSERT_EQ(login_result.get_age(), person.get_age());
}

TEST_F (TestApp, test3) {
    database::Person person;
    person.login() = "333-333-333";
    person.first_name() = "Alexey";
    person.last_name() = "Vorobev";
    person.age() = 22;
    person.save_to_mysql();
    database::Person login_result = database::Person::read_by_login(person.get_login());
    ASSERT_EQ(login_result.get_first_name(), person.get_first_name());
    ASSERT_EQ(login_result.get_last_name(), person.get_last_name());
    ASSERT_EQ(login_result.get_age(), person.get_age());
}

TEST_F (TestApp, test4) {
    database::Person person;
    person.login() = "54321";
    person.first_name() = "Vlad";
    person.last_name() = "Petrushin";
    person.age() = 22;
    person.save_to_mysql();
    database::Person login_result = database::Person::read_by_login(person.get_login());
    ASSERT_EQ(login_result.get_first_name(), person.get_first_name());
    ASSERT_EQ(login_result.get_last_name(), person.get_last_name());
    ASSERT_EQ(login_result.get_age(), person.get_age());
}

TEST_F (TestApp, test5) {
    database::Person person;
    person.login() = "12345";
    person.first_name() = "Alexey";
    person.last_name() = "Vinnikov";
    person.age() = 22;
    person.save_to_mysql();
    database::Person login_result = database::Person::read_by_login(person.get_login());
    ASSERT_EQ(login_result.get_first_name(), person.get_first_name());
    ASSERT_EQ(login_result.get_last_name(), person.get_last_name());
    ASSERT_EQ(login_result.get_age(), person.get_age());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}