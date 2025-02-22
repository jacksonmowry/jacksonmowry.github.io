
using System.ComponentModel.DataAnnotations;

namespace App.Areas.Identity;

/// <summary>
/// Password hashing algorithms supported in this app.
/// </summary>
public enum PasswordHasherAlgorithm {
    [Display(Name = "Iterative")] Iterative,
    [Display(Name = "PBKDF2")] PBKDF2,
    [Display(Name = "BCrypt")] BCrypt,
    [Display(Name = "Argon2id")] Argon2id
}
