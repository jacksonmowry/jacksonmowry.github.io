using System.Security.Cryptography;
using Microsoft.AspNetCore.Identity;
using Konscious.Security.Cryptography;

namespace App.Areas.Identity;

/// <summary>
/// Password hasher backed by Argon2id.
/// </summary>
/// <remarks>
/// For reference, consider the <see href="https://github.com/aspnet/AspNetIdentity/blob/main/src/Microsoft.AspNet.Identity.Core/PasswordHasher.cs">default implementation</see>
/// </remarks>
internal class Argon2idHasher : IPasswordHasher<IdentityUser> {

    /// <summary>
    /// Hash a password using Argon2id.
    /// </summary>
    /// <param name="password">Password to hash.</param>
    /// <returns>String containing all the information needed to verify the password in the future.</returns>
    public string HashPassword(IdentityUser user, string password) {
        // todo: Use a random 32-byte salt. Use a 32-byte digest.
        // todo: Degrees of parallelism is 8, iterations is 4, and memory size is 128MB.
        // todo: Encode as "Base64(salt):Base64(digest)"
        var argon2 = new Argon2id(Utils.Encoding.GetBytes(password));
        byte[] salt = RandomNumberGenerator.GetBytes(32);

        argon2.DegreeOfParallelism = 8;
        argon2.MemorySize = 125000;
        argon2.Iterations = 4;
        argon2.Salt = salt;

        return Utils.EncodeSaltAndDigest(salt, argon2.GetBytes(32));
    }

    /// <summary>
    /// Verify that a password matches the hashed password.
    /// </summary>
    /// <param name="hashedPassword">Hashed password value stored when registering.</param>
    /// <param name="providedPassword">Password provided by user in login attempt.</param>
    /// <returns></returns>
    public PasswordVerificationResult VerifyHashedPassword(IdentityUser user, string hashedPassword, string providedPassword) {
        // todo: Verify that the given password matches the hashedPassword (as originally encoded by HashPassword)
        (byte[] salt, byte[] digest) = Utils.DecodeSaltAndDigest(hashedPassword);

        var argon2 = new Argon2id(Utils.Encoding.GetBytes(providedPassword));

        argon2.DegreeOfParallelism = 8;
        argon2.MemorySize = 125000;
        argon2.Iterations = 4;
        argon2.Salt = salt;

        Console.WriteLine(System.Convert.ToBase64String(digest));
        Console.WriteLine(System.Convert.ToBase64String(argon2.GetBytes(32)));
        if (!digest.SequenceEqual(argon2.GetBytes(32))) {
            return PasswordVerificationResult.Failed;
        }

        return PasswordVerificationResult.Success;
    }

}
