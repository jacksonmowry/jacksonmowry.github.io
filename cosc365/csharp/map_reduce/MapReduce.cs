public partial class MapReduce<T> {
    private List<T> data;
    public int Count;

    public MapReduce() {
        data = new List<T>();
        Count = 0;
    }

    public void Add(T input_data) {
        data.Add(input_data);
        Count++;
    }

    public T this[int i] {
        get => data[i];
    }
}
