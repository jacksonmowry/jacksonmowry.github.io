using System.Security.Cryptography;
using Microsoft.AspNetCore.Identity;

namespace App.Areas.Identity;

/// <summary>
/// Password hasher backed by iterative SHA256 hashing.
/// </summary>
/// <remarks>
/// For reference, consider the <see href="https://github.com/aspnet/AspNetIdentity/blob/main/src/Microsoft.AspNet.Identity.Core/PasswordHasher.cs">default implementation</see>
/// </remarks>
internal class IterativeHasher : IPasswordHasher<IdentityUser>
{

    /// <summary>
    /// Hash a password using iterative SHA256 hashing.
    /// </summary>
    /// <param name="password">Password to hash.</param>
    /// <returns>String containing all the information needed to verify the password in the future.</returns>
    public string HashPassword(IdentityUser user, string password)
    {
        // todo: Use a random 32-byte salt. Use a 32-byte digest.
        // todo: Use 100,000 iterations and the SHA256 algorithm.
        // todo: Encode as "Base64(salt):Base64(digest)"
        byte[] salt = RandomNumberGenerator.GetBytes(32);

        byte[] password_bytes = System.Text.Encoding.ASCII.GetBytes(password);
        byte[] digest = new byte[password_bytes.Length + salt.Length];
        Buffer.BlockCopy(salt, 0, digest, 0, salt.Length);
        Buffer.BlockCopy(password_bytes, 0, digest, salt.Length, password_bytes.Length);


        using (var sha = SHA256.Create())
        {
            for (int i = 0; i < 100000; i++)
            {
                digest = sha.ComputeHash(digest);
            }
        }


        return Utils.EncodeSaltAndDigest(salt, digest);
    }

    /// <summary>
    /// Verify that a password matches the hashed password.
    /// </summary>
    /// <param name="hashedPassword">Hashed password value stored when registering.</param>
    /// <param name="providedPassword">Password provided by user in login attempt.</param>
    /// <returns></returns>
    public PasswordVerificationResult VerifyHashedPassword(IdentityUser user, string hashedPassword, string providedPassword)
    {
        // todo: Verify that the given password matches the hashedPassword (as originally encoded by HashPassword)
        (byte[] salt, byte[] digest) = Utils.DecodeSaltAndDigest(hashedPassword);

        byte[] password_bytes = System.Text.Encoding.ASCII.GetBytes(providedPassword);
        byte[] new_digest = new byte[password_bytes.Length + salt.Length];
        Buffer.BlockCopy(salt, 0, new_digest, 0, salt.Length);
        Buffer.BlockCopy(password_bytes, 0, new_digest, salt.Length, password_bytes.Length);
        using (var sha = SHA256.Create())
        {
            for (int i = 0; i < 100000; i++)
            {
                new_digest = sha.ComputeHash(new_digest);
            }
        }

        if (!digest.SequenceEqual(new_digest)) {
            return PasswordVerificationResult.Failed;
        }

        return PasswordVerificationResult.Success;
    }

}
