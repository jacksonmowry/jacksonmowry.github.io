
using System.ComponentModel.DataAnnotations;
using Microsoft.AspNetCore.Identity;

namespace App.Areas.Identity.Data;

/// <summary>
/// Extends the default user class to also track the password hasher used by this user.
/// </summary>
public class ApplicationUser : IdentityUser {

    [Required]
    public PasswordHasherAlgorithm PasswordHasherAlgorithm { get; set; }

}