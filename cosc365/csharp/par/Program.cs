using System;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;

class Test
{
    private static Object _mut = new object();
    private static Barrier _bar = new Barrier(4);

    public static void Func(object? o)
    {
        var t_num = o as int?;
        if (t_num != null)
        {
            Console.WriteLine($"Thread started... {t_num}");
            lock (_mut)
            {
                Thread.Sleep(1000);
            }
            Console.WriteLine($"Thread done... {t_num}");
        }
    }

    public static void Main()
    {
        Random r = new Random();
        Action a = () =>
        {
            Console.WriteLine("Doing work....");
            Thread.Sleep(r.Next(500, 4000));
            Console.WriteLine("Work done, waiting on barrier....");
            _bar.SignalAndWait();
            Console.WriteLine("Barrier signalled, finishing....");
        };

        for (int i = 0; i < 5; i++)
        {
            Thread t = new Thread(new ThreadStart(a));
            t.Start();
        }

        // int i;
        // List<Thread> mythreads = new List<Thread>();

        // for (i = 0; i < 4; i++)
        // {
        //     Thread t = new Thread(new ParameterizedThreadStart(Func));
        //     mythreads.Add(t);
        //     t.Start(i + 1);
        // }

        // Console.WriteLine("Threads created...");
        // Console.WriteLine("Threads started...");

        // foreach (var t in mythreads)
        // {
        //     t.Join();
        // }

        // Console.WriteLine("Threads joined...");
    }
}
