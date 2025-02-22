#region Assembly System.Security.Cryptography, Version=7.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a
// System.Security.Cryptography.dll
#endregion

#nullable enable

using System.Diagnostics.CodeAnalysis;

namespace System.Security.Cryptography
{
    //
    // Summary:
    //     Provides functionality for generating random values.
    public abstract class RandomNumberGenerator : IDisposable
    {
        //
        // Summary:
        //     Initializes a new instance of System.Security.Cryptography.RandomNumberGenerator.
        protected RandomNumberGenerator();

        //
        // Summary:
        //     Creates an instance of the default implementation of a cryptographic random number
        //     generator that can be used to generate random data.
        //
        // Returns:
        //     A new instance of a cryptographic random number generator.
        public static RandomNumberGenerator Create();
        //
        // Summary:
        //     Creates an instance of the specified implementation of a cryptographic random
        //     number generator.
        //
        // Parameters:
        //   rngName:
        //     The name of the random number generator implementation to use.
        //
        // Returns:
        //     A new instance of a cryptographic random number generator.
        [Obsolete("Cryptographic factory methods accepting an algorithm name are obsolete. Use the parameterless Create factory method on the algorithm type instead.", DiagnosticId = "SYSLIB0045", UrlFormat = "https://aka.ms/dotnet-warnings/{0}")]
        [RequiresUnreferencedCode("The default algorithm implementations might be removed, use strong type references like 'RSA.Create()' instead.")]
        public static RandomNumberGenerator? Create(string rngName);
        //
        // Summary:
        //     Fills a span with cryptographically strong random bytes.
        //
        // Parameters:
        //   data:
        //     The span to fill with cryptographically strong random bytes.
        public static void Fill(Span<byte> data);
        //
        // Summary:
        //     Creates an array of bytes with a cryptographically strong random sequence of
        //     values.
        //
        // Parameters:
        //   count:
        //     The number of bytes of random values to create.
        //
        // Returns:
        //     An array populated with cryptographically strong random values.
        //
        // Exceptions:
        //   T:System.ArgumentOutOfRangeException:
        //     count is less than zero.
        public static byte[] GetBytes(int count);
        //
        // Summary:
        //     Generates a random integer between 0 (inclusive) and a specified exclusive upper
        //     bound using a cryptographically strong random number generator.
        //
        // Parameters:
        //   toExclusive:
        //     The exclusive upper bound of the random range.
        //
        // Returns:
        //     A random integer between 0 (inclusive) and toExclusive (exclusive).
        //
        // Exceptions:
        //   T:System.ArgumentOutOfRangeException:
        //     The toExclusive parameter is less than or equal to 0.
        public static int GetInt32(int toExclusive);
        //
        // Summary:
        //     Generates a random integer between a specified inclusive lower bound and a specified
        //     exclusive upper bound using a cryptographically strong random number generator.
        //
        //
        // Parameters:
        //   fromInclusive:
        //     The inclusive lower bound of the random range.
        //
        //   toExclusive:
        //     The exclusive upper bound of the random range.
        //
        // Returns:
        //     A random integer between fromInclusive (inclusive) and toExclusive (exclusive).
        //
        //
        // Exceptions:
        //   T:System.ArgumentOutOfRangeException:
        //     The toExclusive parameter is less than or equal to the fromInclusive parameter.
        public static int GetInt32(int fromInclusive, int toExclusive);
        //
        // Summary:
        //     When overridden in a derived class, releases all resources used by the current
        //     instance of the System.Security.Cryptography.RandomNumberGenerator class.
        public void Dispose();
        //
        // Summary:
        //     When overridden in a derived class, fills an array of bytes with a cryptographically
        //     strong random sequence of values.
        //
        // Parameters:
        //   data:
        //     The array to fill with cryptographically strong random bytes.
        public abstract void GetBytes(byte[] data);
        //
        // Summary:
        //     Fills the specified byte array with a cryptographically strong random sequence
        //     of values.
        //
        // Parameters:
        //   data:
        //     The array to fill with cryptographically strong random bytes.
        //
        //   offset:
        //     The index of the array to start the fill operation.
        //
        //   count:
        //     The number of bytes to fill.
        //
        // Exceptions:
        //   T:System.ArgumentNullException:
        //     data is null.
        //
        //   T:System.ArgumentOutOfRangeException:
        //     offset or count is less than 0
        //
        //   T:System.ArgumentException:
        //     offset plus count exceeds the length of data.
        public virtual void GetBytes(byte[] data, int offset, int count);
        //
        // Summary:
        //     Fills a span with cryptographically strong random bytes.
        //
        // Parameters:
        //   data:
        //     The span to fill with cryptographically strong random bytes.
        public virtual void GetBytes(Span<byte> data);
        //
        // Summary:
        //     When overridden in a derived class, fills an array of bytes with a cryptographically
        //     strong random sequence of nonzero values.
        //
        // Parameters:
        //   data:
        //     The array to fill with cryptographically strong random nonzero bytes.
        public virtual void GetNonZeroBytes(byte[] data);
        //
        // Summary:
        //     Fills a byte span with a cryptographically strong random sequence of nonzero
        //     values.
        //
        // Parameters:
        //   data:
        //     The span to fill with cryptographically strong random nonzero bytes.
        public virtual void GetNonZeroBytes(Span<byte> data);
        //
        // Summary:
        //     When overridden in a derived class, releases the unmanaged resources used by
        //     the System.Security.Cryptography.RandomNumberGenerator and optionally releases
        //     the managed resources.
        //
        // Parameters:
        //   disposing:
        //     true to release both managed and unmanaged resources; false to release only unmanaged
        //     resources.
        protected virtual void Dispose(bool disposing);
    }
}