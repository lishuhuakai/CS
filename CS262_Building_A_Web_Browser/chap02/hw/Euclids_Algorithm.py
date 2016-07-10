# Euclid's Algorithm
#
# Format: Submit JavaScript Code
#
# In mathematics, because 8 divides 24 evenly, we say that 8 is a
# *divisor* of 24. When computing, it is often useful to find the
# largest divisor that two numbers have in common. For example, 36
# and 24 are both divisible by 2, 3, 4, and 12: the greatest
# divisor that they have in common is 12. It turns out that finding
# common divisors for numbers is critical for modern cryptography,
# including public-key crypto systems such as RSA: a backbone of internet
# commerce.
#
# Perhaps the oldest algorithm known -- ever! -- is for computing the
# greatest common divisor of two positive numbers. It is attributed to the
# Greek mathematician Euclid around 300 BCE. Here's how it goes:
#
# You are computing the greatest common divisor ("gcd") of two positive
# integers called "a" and "b". The gcd can be computed recursively (or
# iteratively) using the following three rules:
#
#       gcd(a,b) = a                    if a == b
#       gcd(a,b) = gcd(a-b,b)           if a > b
#       gcd(a,b) = gcd(a,b-a)           if a < b
#
# Write a JavaScript (_not_ Python) program that declares a function called gcd
# that accepts two positive integer arguments a and b and returns their greatest
# common divisor. Store your function in a variable called javascriptcode.
#
# We will return anything printed out when you hit submit as we execute the
# JavaScript behind the scenes.

javascriptcode="""
function gcd(a,b) {
    //Write Code Here
    if (a == b) return a;
    else if (a > b) return gcd(a-b, b);
    else return gcd(a, b-a)
}

write( gcd(24,8) == 8 );
write(" ");
write( gcd(1362, 1407) ); // Empress Xu (Ming Dynasty) wrote biographies
write(" ");
write( gcd(1875, 1907) ); // Qiu Jin, feminist, revolutionary, and writer
write(" ");
write( gcd(45,116) ); // Ban Zhao, first known female Chinese historian
"""