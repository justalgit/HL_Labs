#include <gtest/gtest.h>
#include "config/config.h"
#include "database/database.h"
#include "database/person.h"
#include <Poco/Data/SessionFactory.h>
using Poco::Data::Session;
using Poco::Data::Statement;
using namespace Poco::Data::Keywords;

class TestApp : public ::testing::Test {
protected:
    TestApp() {
        Config::get().host() = "127.0.0.1";
        Config::get().database() = "itlabs";
        Config::get().port() = "8080";
        Config::get().login() = "lab";
        Config::get().password() = "12345";
    }
    ~TestApp() {
        Poco::Data::Session session = database::Database::get().create_session();
        Statement reset_data(session);
        reset_data << "DELETE FROM Person WHERE id > 100000", now;
        Statement reset_increment(session);
        reset_increment << "ALTER TABLE Person AUTO_INCREMENT=100001", now;
    }
     void SetUp() {}
     void TearDown() {}

protected:
};

TEST_F(TestApp, TestPerson) {

    database::Person person;

    //POST tests
    person.login() = "111";
    person.first_name() = "Anton";
    person.last_name() = "Larin";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "inserted:100001\n");

    person.login() = "222";
    person.first_name() = "A";
    person.last_name() = "L";
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "inserted:100002\n");

    person.login() = "333-333-333";
    person.first_name() = "Alexey";
    person.last_name() = "Vorobev";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "inserted:100003\n");

    person.login() = "54321";
    person.first_name() = "Vlad";
    person.last_name() = "Petrushin";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "inserted:100004\n");

    person.login() = "12345";
    person.first_name() = "Alexey";
    person.last_name() = "Vinnikov";
    person.age() = 22;
    testing::internal::CaptureStdout();
    person.save_to_mysql();
    ASSERT_EQ(testing::internal::GetCapturedStdout(), "inserted:100005\n");

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
    ASSERT_EQ(name_result2.at(0).get_last_name(), "Vorobev");

    auto full_query = database::Person::read_all();
    ASSERT_EQ(full_query.size(), 100005);
    ASSERT_EQ(full_query.at(100000).get_login(), "111");
    ASSERT_EQ(full_query.at(100003).get_last_name(), "Petrushin");
    ASSERT_EQ(full_query.at(100004).get_last_name(), "Vinnikov");
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}