public partial class MapReduce<T> {
    public delegate T MapFunction(T func);

    public void Map(MapFunction mf) {
        Parallel.For(0, data.Count(), index => data[index] = mf(data[index]));
    }
}
