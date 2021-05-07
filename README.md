# Лабораторные работы №1-3 (Управление IT-проектами) #

Выполнил: Ларин Антон, М8О-103М-20

## Сборка проекта ##

```bash
cmake configure .
cmake --build ./
```

## Запуск Docker-окружения ##

```bash
sudo docker-compose up -d
```

## Создание скрипта для заполнения шардов ##

Искуственно сгенерированный датасет, использовавшийся в лабораторных работах 1 и 2, необходимо разделить на 3 запроса для каждого из шардов:

```bash
./data_shard_splitter
```

Данный исполняемый файл генерирует скрипт shard_fill.sql.

## Настройка базы данных ##

1. Создание таблицы Person на каждом из трех шардов.
2. Заполнение базы данных с помощью сгенерированного скрипта.

```bash
mysql -u test -p pzjqUkMnc7vfNHET -h 127.0.0.1 -P 6033 --comments
mysql> source db_scripts/shard_init.sql;
mysql> source db_scripts/shard_fill.sql;
```

Удаление данных с узлов выполняется с помощью следующей команды:

```bash
mysql> source db_scripts/shard_cleanup.sql;
```

## Запуск сервера ##

Для запуска сервера следует выполнить команду:

```bash
sudo sh ./start.sh
```

Сервер работает на порту 8080.

## Модульное тестирование (GTests) ##

Запуск модульных тестов осуществляется следующей командой:

```bash
./gtests
```

Для тестирования используется исходная таблица Person с последующей очисткой данных в каждом шарде по окончании работы gtests.

## Нагрузочное тестирование (wrk) ##

Нагрузочное тестирование производилось для 1, 2, 6 и 10 потоков при 50 подключениях в течение 30 секунд:

```bash
wrk -t <количество потоков> -c 50 -d 30s http://localhost:8080/person?login=731-07-5834
```

- Threads - количество потоков;
- Requests/sec - количество запросов в секунду;
- Latency(ms) - задержка в миллисекундах.

Threads | Requests/sec   | Latency(ms)
---     | ---            | ---
1       | 2908.16        | 6.19
2       | 3062.89        | 5.16
6       | 2874.30        | 5.58
10      | 2880.73        | 5.65

## Остановка работы Docker-окружения ##

```bash
sudo docker-compose stop
```
