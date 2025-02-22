using System;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;

class Test {
    private static Semaphore _sem = new Semaphore(0, 2);
    public static void Func(object? o) {
        var t_num = o as int?;
        if (t_num != null) {
            Console.WriteLine($"Thread started... {t_num}");
            _sem.WaitOne();
            Thread.Sleep(1000);
            _sem.Release();
            Console.WriteLine($"Thread done... {t_num}");
        }
    }

    public static void Main() {
        int i;
        List<Thread> mythreads = new List<Thread>();

        for (i = 0; i < 4; i++) {
            Thread t = new Thread(new ParameterizedThreadStart(Func));
            mythreads.Add(t);
            t.Start(i+1);
        }

        Console.WriteLine("Threads created...");
        Console.WriteLine("Threads started...");

        foreach(var t in mythreads) {
            t.Join();
        }

        Console.WriteLine("Threads joined...");
    }
}
