// Jackson Mowry
public partial class MapReduce<T> {
    private List<T> data;

    public MapReduce() {
        data = new List<T>();
    }

    public void Add(T input_data) {
        data.Add(input_data);
    }

    public T this[int i] {
        get => data[i];
    }

    public int Count {
        get => data.Count();
    }
}
