# Лабораторные работы №1 и №2 (Управление IT-проектами) #

Выполнил: Ларин Антон, М8О-103М-20

## Сборка проекта ##

```bash
cmake configure .
cmake --build ./
```

## Настройка базы данных ##

1. Для создания тестового пользователя с именем lab и базы данных itlabs с необходимой таблицей Person необходимо выполнить команды из файла db_scripts/db_creation.sql.
2. Скрипт заполнения базы данных сгенерированными записями (100 тысяч строк) записан в файле db_scripts/data_generation.sql.

Выполнение скрипта из файла:

```bash
source <имя файла>.sql;
```

## Запуск сервера ##

Для запуска сервера следует выполнить команду:

```bash
sudo sh ./start.sh
```

Сервер работает на порту 8080.

## Запуск Apache Ignite ##

```bash
sudo docker-compose up -d
```

## Тестирование с помощью gtest ##

Запуск модульных тестов осуществляется командой:

```bash
./gtests
```

## Нагрузочное тестирование с помощью wrk ##

Нагрузочное тестирование производилось для 1, 2, 6 и 10 потоков при 50 подключениях в течение 30 секунд:

```bash
wrk -t 2 -c 50 -d 30s http://192.168.31.116:8080/person?login=411-88-7854
```

- Threads - количество потоков;
- Requests/sec - количество запросов в секунду;
- Latency(ms) - задержка в миллисекундах.

Без кеширования данных (ЛР №1):

Threads | Requests/sec | Latency(ms)
---     | ---          | ---
1       | 282.14       | 56.60
2       | 301.79       | 53.09
6       | 334.18       | 47.90
10      | 303.34       | 52.67

С кешированием данных с помощью Apache Ignite (ЛР №2):

Threads | Requests/sec   | Latency(ms)
---     | ---            | ---
1       | 3183.93        | 4.91
2       | 3197.09        | 4.89
6       | 3047.05        | 5.27
10      | 2989.98        | 6.15
