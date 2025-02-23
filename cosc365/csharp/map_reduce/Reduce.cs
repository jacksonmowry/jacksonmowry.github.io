// Jackson Mowry
public partial class MapReduce<T>
{
    public delegate T ReduceFunction(T left, T right);

    public T Reduce(ReduceFunction rf)
    {
        T accumulator = data[0];
        for (int i = 1; i < data.Count(); i++)
        {
            accumulator = rf(accumulator, data[i]);
        }

        return accumulator;
    }

    public async Task<T> ReduceAsync(ReduceFunction rf)
    {
        return await Task.Run(() =>
        {
            return Reduce(rf);
        });
    }
}
