# Бугаков Максим БПИ226
## ДЗ 3
**Проделанная работа:**
1. Реализовал две рекурсивные функции: факториала и Фибоначчи.
2. Написал функцию для процесса-ребёнка, которая проверяет факториал на переполнение и выводит результат.
3. В основной функции программы создаю процесс-ребёнка.
4. В блоке if, если ID процесса равен 0, это означает, что выполняется процесс-ребёнок, поэтому вызываю child_process(n).
5. В противном случае, вычисляю число Фибоначчи, так как это процесс-родитель.
6. Добавил выводы ID процессов, родительского и ребёнка.
7. Создал отдельную функцию для вывода содержимого текущего каталога, которую вызывает процесс-родитель.
