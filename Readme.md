# OpenFlight Reader

Простая утилита для чтения и анализа файлов в формате OpenFlight (.flt).

## Описание

Программа читает бинарные файлы формата OpenFlight (версия 16.4) и выводит иерархию элементов, а также информацию о полигонах.

## Требования

- C++23 компилятор (g++ или clang++)

## Сборка

```bash
make
```

## Запуск

```bash
make run
```

## Очистка проекта

```bash
make clean
```

## Вывод программы

Программа выводит древовидную структуру модели с отступами:

* ID элементов (db, g1, g2, o1, o2)

* Информацию о полигонах (polygon_1, polygon_2, polygon_3)

* Индексы цветов и материалов для каждого полигона


## Формат вывода
```bash
ID: db
    ID: g1
        ID: g2
            ID: o1
                ID: 'polygon_1', Color Name Index: 0, Material Index: -1
            ID: o2
                ID: 'polygon_2', Color Name Index: 0, Material Index: -1
                ID: 'polygon_3', Color Name Index: 0, Material Index: -1
```
