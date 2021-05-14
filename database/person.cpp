#include "person.h"
#include "cache.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <cppkafka/cppkafka.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{
    void Person::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            //*
            Statement drop_stmt(session);
            drop_stmt << "DROP TABLE IF EXISTS Person", now;
            //*/

            // (re)create table
            Statement create_stmt(session);
            create_stmt << "CREATE TABLE IF NOT EXISTS 'Person' ('id' INT NOT NULL AUTO_INCREMENT,"
                        << "'login' VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "'first_name' VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "'last_name' VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                        << "'age' TINYINT UNSIGNED NULL"
                        << "PRIMARY KEY ('id'),UNIQUE KEY 'login_hash' ('login'), KEY 'fn' ('first_name'),KEY 'ln' ('last_name'));",
                    now;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr Person::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("login", _login);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("age", _age);

        return root;
    }

    Person Person::fromJSON(const std::string &str)
    {
        Person person;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        person.id() = object->getValue<long>("id");
        person.login() = object->getValue<std::string>("login");
        person.first_name() = object->getValue<std::string>("first_name");
        person.last_name() = object->getValue<std::string>("last_name");
        person.age() = object->getValue<unsigned char>("age");

        return person;
    }

    Person Person::read_by_login(std::string login)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            std::string sql_request = "SELECT id, login, first_name, last_name, age FROM Person where login=? ";
            sql_request += database::Database::sharding_hint(login, 3);
            std::cout << sql_request << std::endl;
            Person p;
            select << sql_request,
                    into(p._id),
                    into(p._login),
                    into(p._first_name),
                    into(p._last_name),
                    into(p._age),
                    use(login),
                    range(0, 1); //  iterate over result set one row at a time
            select.execute();
            return p;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<Person> Person::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            std::vector<Person> result;
            Person p;
            std::string sql_request = "SELECT id, login, first_name, last_name, age FROM Person";
            std::string sql_request_to_shard;
            int max_shard = 3;

            for(int i = 0; i < max_shard; i++) {
            	Statement select(session);
                sql_request_to_shard = sql_request + " -- sharding:" + std::to_string(i);
                Person p;
                select << sql_request_to_shard,
                        into(p._id),
                        into(p._login),
                        into(p._first_name),
                        into(p._last_name),
                        into(p._age),
                        range(0, 1); //  iterate over result set one row at a time

                while (!select.done()) {
                    select.execute();
                    if(!std::empty(p._login))
                    	result.push_back(p);
                }
                std::cout << sql_request_to_shard << std::endl;
            }

            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<Person> Person::search(std::string first_name, std::string last_name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            std::vector<Person> result;
            first_name += "%";
            last_name += "%";
            std::string sql_request = "SELECT id, login, first_name, last_name, age FROM Person WHERE first_name LIKE ? and last_name LIKE ?";
            std::string sql_request_to_shard;
            int max_shard = 3;

            for(int i = 0; i < max_shard; i++) {
            	Statement select(session);
                sql_request_to_shard = sql_request + " -- sharding:" + std::to_string(i);
                Person p;
                select
                        << sql_request_to_shard,
                        into(p._id),
                        into(p._login),
                        into(p._first_name),
                        into(p._last_name),
                        into(p._age),
                        use(first_name),
                        use(last_name),
                        range(0, 1); //  iterate over result set one row at a time

                while (!select.done()) {
                    select.execute();
                    if(!std::empty(p._login))
                    	result.push_back(p);
                }
                std::cout << sql_request_to_shard << std::endl;
            }
            return result;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void Person::save_to_mysql()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            std::string sql_request = "INSERT INTO Person (login, first_name, last_name, age) VALUES(?, ?, ?, ?) ";
            std::string comment = database::Database::sharding_hint(_login, 3);
            sql_request += comment;
            //std::cout << comment << std::endl;

            insert << sql_request,
                    use(_login),
                    use(_first_name),
                    use(_last_name),
                    use(_age),
                    now;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }

        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << e.message() << std::endl;
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    //Cache section

    void Person::warm_up_cache()
    {
        std::cout << "warming up persons cache ... ";
        auto array = read_all();
        long count = 0;
        for (auto &p : array)
        {
            p.save_to_cache();
            ++count;
        }
        std::cout << "done: " << count << std::endl;
    }

    Person Person::read_from_cache_by_login(std::string login)
    {
        try
        {
            std::string result;
            if (database::Cache::get().get(login, result))
                return fromJSON(result);
            else
                throw std::logic_error("key not found in the cache");
        }
        catch (std::exception &err)
        {
            //std::cout << "error:" << err.what() << std::endl;
            throw;
        }
    }

    void Person::save_to_cache()
    {
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        database::Cache::get().put(_login, message);
    }

    size_t Person::size_of_cache(){
        return database::Cache::get().size();
    }

    //Queue section

    void Person::send_to_queue()
    {
        cppkafka::Configuration config = {
                {"metadata.broker.list", Config::get().get_queue_host()}};

        cppkafka::Producer producer(config);
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        producer.produce(cppkafka::MessageBuilder(Config::get().get_queue_topic()).partition(0).payload(message));
        producer.flush();
    }

    long Person::get_id() const
    {
        return _id;
    }

    const std::string &Person::get_login() const
    {
        return _login;
    }

    const std::string &Person::get_first_name() const
    {
        return _first_name;
    }

    const std::string &Person::get_last_name() const
    {
        return _last_name;
    }

    unsigned char Person::get_age() const
    {
        return _age;
    }

    long &Person::id()
    {
        return _id;
    }

    std::string &Person::login()
    {
        return _login;
    }
    std::string &Person::first_name()
    {
        return _first_name;
    }

    std::string &Person::last_name()
    {
        return _last_name;
    }

    unsigned char &Person::age()
    {
        return _age;
    }
}