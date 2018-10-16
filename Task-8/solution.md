	Распределенный счетчик - 2+3 балла

    > 1) Будет ли такая реализация линеаризуемой? Сможет ли программа отличить такой счетчик от атомарного?

    *Нет, не будет*. Приведем пример нелианеризуемой истории. Пусть есть два потока-продьюсера, а запускать `Get` будем один раз.
    Пусть `Add(t, x)` - добавление потоком `t` числа `x` в свой счетчик. Пусть `Get(t)=S` - событие "Поток, вызвавший `Get` посмотрел счетчик `t` и вернул `S`".
    Тогда история будет такой:

    * `Get` - Start
    * `Get(1)=0`
    * `Add(1, 10)`
    * `Add(2, 20)`
    * `Get(2)=20`
    * `Sum=20`
    Таким образом, `Get` не прочитал первое изменение но прочитал второе. Если бы такая история была линеаризуемой, `Get` должна была бы вернуть 30, так как частичный порядок должен сохраниться, а операция `Add(1,..)` шла раньше операции `Add(2,..)`.

    > 2) Будет ли счетчик линеаризуемым, если заменить операцию Add(x) на операцию Increment(), которая увеличивает V[t] на +1?

    *Да, будет*.

    Воспользуемся тем, что Increment() увеличивает свою переменную на единицу. Из этого следует, что в любой корректной истории счетчик `Get` вернет значение не большее, чем количество вызовов `Increment`, инициированных до окончания `Get`. Ну а тогда схема лианеризации становится понятной. Пусть есть несколько потоков, представленных каждый на своей временной прямой. Тогда активность каждого потока в истории представляется набором непересекающихся отрезков.

    * **Шаг первый:** возьмем за точки лианеризации у `Increment`-ов времена, когда соответствующая атомарная переменная была изменена. (Немного понудним: если две точки у разных потоков находятся на одном уровне, сдвинем немного одну из них) - понятно, что так сделать мы можем.
    * **Шаг второй:** возьмем функцию `Sum(t)`, равную сумме количеств точек лианеризации, расположенных левее времени `t`.
    * **Шаг третий:** Пусть теперь в рассматриваемой истории поток `Get` вернул значение `S`. Как уже было упомянуто ранее, `S <= количество инкрементов`. Это значит, что существует `t` такое, что `Sum(t)=S`. Тогда скажем, что `Get` выполнялся ровно во время `t`, а сами потоки упорядочим по их точкам линеаризации.
    *История линеаризована.*

	Поиск без блокировок в оптимистичном сортированном списке - 2 балла
    [Решение](https://ibin.co/3MTTSXuuj1JR.jpg)

	Очередь - 2 балла

    *Нет, не будет*
   
    Приведем пример.
    Пусть первый поток заходит в enqueue и сразу же останавливается. Второй поток тем временем успешно проходит дальше и линкуется. 
    При этом вызов contains (ну или dequeue) возвращает none. Такая история не линеаризуема, т.к. если бы contains (dequeue) увидел второй, то он увидел бы и первый.

