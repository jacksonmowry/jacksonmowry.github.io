using System.Text;

namespace App.Areas.Identity;

internal static class Utils {

    /// <summary>
    /// Encoding used to convert strings to and from bytes.
    /// </summary>
    public static Encoding Encoding { get => Encoding.ASCII; }

    /// <summary>
    /// Encodes a salt and a digest into a string.
    /// </summary>
    /// <param name="salt">Salt to encode.</param>
    /// <param name="digest">Digest to encode.</param>
    /// <returns>Encoded salt and digest.</returns>
    public static string EncodeSaltAndDigest(byte[] salt, byte[] digest) {
        return $"{System.Convert.ToBase64String(salt)}:{System.Convert.ToBase64String(digest)}";
    }

    /// <summary>
    /// Decodes a salt and a digest from a string.
    /// </summary>
    /// <param name="salt">Salt to decode.</param>
    /// <param name="digest">Digest to decode.</param>
    /// <returns>Decoded salt and digest.</returns>
    public static (byte[], byte[]) DecodeSaltAndDigest(string value) {
        string[] split = value.Split(':');
        return (System.Convert.FromBase64String(split[0]), System.Convert.FromBase64String(split[1]));
    }

}
