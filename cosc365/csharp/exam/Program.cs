using System;
using System.IO;
using System.Collections.Generic;

class Cart
{
    public static void Main(string[] nothing)
    {
        //System.Console.Write("Enter shopping cart file: ");
        Cart cart = new Cart();

        try
        {
            string? line = System.Console.ReadLine();
            if (line == null)
            {
                throw new IOException();
            }
            using (StreamReader sr = new StreamReader(line))
            {
                string? input;

                while ((input = sr.ReadLine()) != null)
                {
                    string[] splitted = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);
                    Item item = new Item(System.Convert.ToDouble(splitted[0]), splitted[1]);
                    bool found = false;
                    for (int i = 0; i < cart._items.Count(); i++)
                    {
                        if (cart._items[i].item_name.Equals(item.item_name))
                        {
                            cart._items[i].cost += item.cost;
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        cart._items.Add(item);
                    }

                }

                cart._items.Sort();

                for (int i = 0; i < cart._items.Count(); i++)
                {
                    System.Console.WriteLine($"{cart._items[i].item_name,-26} ${cart._items[i].cost,7:F2}");
                }

                double total = cart._items.Aggregate(0.0, (acc, x) => acc + x.cost);
                System.Console.WriteLine($"Total = ${total:F2}");
            }
        }
        catch (IOException)
        {
            System.Console.WriteLine("Exception message");
        }

    }
    private List<Item> _items;
    public Cart()
    {
        _items = new List<Item>();
    }
    class Item : IComparable<Item>
    {
        public Item(double c, string input_name)
        {
            cost = c;
            item_name = input_name;
        }

        public double cost;
        public string item_name;

        public int CompareTo(Item? i)
        {
            if (i == null)
            {
                return 1;
            }
            if (i.cost == this.cost) {
                return this.item_name.CompareTo(i.item_name);
            }

            return System.Convert.ToInt32(i.cost - this.cost);
        }
    }
}
