#include <unordered_set>
#include <algorithm>
#include <iterator>

template <typename T, typename Hash = std::hash<T>, typename KeyEqual = std::equal_to<T>, typename Allocator = std::allocator<T>>
class easy_set : public std::unordered_set<T, Hash, KeyEqual, Allocator>
{
public:
    // Inherit constructors from std::unordered_set
    using std::unordered_set<T, Hash, KeyEqual, Allocator>::unordered_set;

    // Set subtraction operator
    easy_set operator-(const easy_set &other) const
    {
        easy_set result = *this;
        for (const auto &item : other)
        {
            result.erase(item);
        }
        return result;
    }

    // Set difference in-place operator
    easy_set &operator-=(const easy_set &other)
    {
        for (const auto &item : other)
        {
            this->erase(item);
        }
        return *this;
    }

    // Set union operator
    easy_set operator+(const easy_set &other) const
    {
        easy_set result = *this;
        result.insert(other.begin(), other.end());
        return result;
    }

    // Set union in-place operator
    easy_set &operator+=(const easy_set &other)
    {
        this->insert(other.begin(), other.end());
        return *this;
    }

    // Set intersection operator
    easy_set operator&(const easy_set &other) const
    {
        easy_set result;
        for (const auto &item : *this)
        {
            if (other.count(item) > 0)
            {
                result.insert(item);
            }
        }
        return result;
    }

    // Set intersection in-place operator
    easy_set &operator&=(const easy_set &other)
    {
        easy_set temp = *this & other;
        *this = std::move(temp);
        return *this;
    }

    // Check if the set contains all elements of another set
    bool contains(const easy_set &other) const
    {
        return std::all_of(other.begin(), other.end(),
                           [this](const T &item)
                           { return this->count(item) > 0; });
    }

    bool has(T element) const
    {
        return this->find(element) != this->end();
    }

    // Symmetric difference operator (elements in either set but not both)
    easy_set operator^(const easy_set &other) const
    {
        easy_set result;
        for (const auto &item : *this)
        {
            if (other.count(item) == 0)
            {
                result.insert(item);
            }
        }
        for (const auto &item : other)
        {
            if (this->count(item) == 0)
            {
                result.insert(item);
            }
        }
        return result;
    }

    // Custom print method for easy debugging
    void print(std::ostream &os = std::cout) const
    {
        os << "{ ";
        for (const auto &item : *this)
        {
            os << item << " ";
        }
        os << "}";
    }

    // Optional: Overload << operator for easy printing
    friend std::ostream &operator<<(std::ostream &os, const easy_set &set)
    {
        set.print(os);
        return os;
    }
};
