using System;
using System.IO;

class LabTest
{
    public static int Main(string[] args) {
        if (args.Length < 1) {
            Console.WriteLine($"Usage: {AppDomain.CurrentDomain.FriendlyName} <filename>");
            return -1;
        }
        DataReader dr;
        try {
            dr = new DataReader(args[0]);
        }
        catch (FileNotFoundException) {
            Console.WriteLine($"{args[0]}: File not found");
            return -2;
        }
        Console.WriteLine($"Done reading {args[0]}, {dr.Count} records read.");

        var mr = new MapReduce<double>();
        foreach (double data in dr) {
            mr.Add(data);
        }
        mr.Map(i => { return (i+i)/2; });
        var rtask = mr.ReduceAsync((accum, cur) => {
            return (accum + cur) / Math.Sqrt(accum + cur);
        });
        ulong iters = 0;
        while (rtask.IsCompleted == false)
             iters++;
        rtask.Wait();
        Console.WriteLine($"Reduce returned value: {rtask.Result:F3}. Reduce took {iters} iterations.");
        return 0;
    }
}
