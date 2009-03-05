#ifndef CONFIG_HPP
#define CONFIG_HPP
/*!
 *\file config.hpp
 * This file contains most of the interface for the external->in memory configuration management interface.
 * There is a Config class which is the main interface to the outside world for loading files, getting and
 * setting values as well as saving the given configuration. This base class is then derived from for
 * the different given backends. This Config class has a root 'Section'. Each section has a list of 
 * subsections and a list of keys. Each key actually contains the value.
 */
// Config Class
// Author: Charles Gruenwald III
#include <vector>
#include <map>
#include <string>
#include <iostream>

#include "key.hpp"
#include "section.hpp"
#include "config_exceptions.hpp"


namespace config
{

    //
    typedef std::vector < std::string > PathElementList;
    typedef std::pair<std::string,std::string> PathPair;

    /*! \brief Config: A class for managing the interface to persistent configuration entries defined at runtime.
     * This class is used to manage a configuration interface.
     * It is the base class for which different back ends will derive from.
     */
    class Config
    {
        public:
            Config(bool case_sensitive = false): m_case_sensitive(case_sensitive), m_root("", case_sensitive){}
            Config(const Section & root, bool case_sensitive = false): m_case_sensitive(case_sensitive), m_root(root, "", case_sensitive){}
            virtual ~Config(){}

            /*! \brief A function for saving the entire configuration
             * tree to the specified path.
             * This function is responsible for walking through the entire
             * configuration tree (starting at the root) and outputing an
             * appropriate text representation than can then be re-read by
             * an appropriate configuration class.
             * Note: In the case of a write-through situation (as is the case
             * with the windows registry), this function is unnecessary.
             */
            virtual void SaveAs(const std::string & path){}

            /*! \brief A function to convert from external representation to in-memory representation
             * This function sets the member variable m_path and then calls the LoadConfig() function which
             * must be implemented by the derived back-end implementation.
             * \param path - This is the path where we load the configuration from. This may either be a registry
             * path for the case of the ConfigRegistry interface or a file path in the case of a ConfigFile interface.
             * \exception FileNotFound - This exception is thrown if the ConfigFile interface is instructed to load
             * a file which does not exist.
             * \exception ParseError - This exception is thrown on a malformed file with the ConfigFile interface.
             */
            void Load(const std::string &path);

            /*! \brief A function which will save the in-memory representation to the external
             * representation that was specified with the Load() function.
             */
            void Save(){SaveAs(m_path);}

            void Clear();

            //! A function that will save a given value to key at the specified path.
            virtual void Set(const std::string & path, const std::string & new_value);

            /*! 
             * \overload virtual void Set(const std::string & path, int new_value);
             */
            virtual void Set(const std::string & path, int new_value);

            /*!
             * \overload virtual void Set(const std::string & path, double new_value);
             */
            virtual void Set(const std::string & path, double new_value);

            //! Returns a reference to the section at the given path.
            const Section & GetSection(const std::string & path);


            //! Returns a reference to the root section of the configuration tree.
            const Section & GetRoot() {return m_root;}

            /*! \brief AddSection() Adds the specified path as a new section, creating each entry
             * in the path along the way.
             */
            const Section & AddSection(const std::string & path);


            /*! \brief Look up the key at the given path, and return the value of that key as a bool.
             * \param path - Path for key to look up
             * \exception KeyNotFound is thrown if the specified path doesn't exist.
             */
            bool GetBool(const std::string & path);

            /*! \brief Look up the key at the given path, return default_val if not found.
             * \param path - Path for key to look up
             * \param default_val - Value to return if the specified key is not found.
             */
            bool GetBool(const std::string & path, bool default_val);

            /*! \brief Look up the key at the given path, and return the value of that key as a bool.
             * \param path - Path for key to look up
             * \exception KeyNotFound is thrown if the specified path doesn't exist.
             */
            int GetInt(const std::string & path);

            /*! \brief Look up the key at the given path, return default_val if not found.
             * \param path - Path for key to look up
             * \param default_val - Value to return if the specified key is not found.
             */
            int GetInt(const std::string & path, int default_val);

            /*! \brief Look up the key at the given path, and return the value of that key as a bool.
             * \param path - Path for key to look up
             * \exception KeyNotFound is thrown if the specified path doesn't exist.
             */
            const std::string GetString(const std::string & path);

            /*! \brief Look up the key at the given path, return default_val if not found.
             * \param path - Path for key to look up
             * \param default_val - Value to return if the specified key is not found.
             */
            const std::string GetString(const std::string & path, const std::string & default_val);

            //! Same as GetString()
            const std::string Get(const std::string &path) { return GetString(path); }
            //! Same as GetString()
            const std::string Get(const std::string &path, const std::string & default_val) { return GetString(path, default_val); }

            /*! \brief Look up the key at the given path, and return the value of that key as a bool.
             * \param path - Path for key to look up
             * \exception KeyNotFound is thrown if the specified path doesn't exist.
             */
            double GetFloat(const std::string & path);

            /*! \brief Look up the key at the given path, return default_val if not found.
             * \param path - Path for key to look up
             * \param default_val - Value to return if the specified key is not found.
             */
            double GetFloat(const std::string & path, double default_val);

            /*! \brief Returns a string representation of the tree starting at the specified section
             * \param current The root of the tree for which we are creating a string representation.
             */
            std::string ShowTree(const Section & current, int depth = 0);

            //! \brief Returns a string representation of the loaded configuration tree
            std::string ShowFullTree() { return ShowTree(m_root); }

            /*! AddKey() Adds the specified path as a new key (with the given value), creating each entry
             * in the path along the way.
             * \param path The path to the new key
             * \param new_key The value for the newly created key
             */
            const Key & AddKey(const std::string & path, const std::string & new_key);
            /*! \overload AddKey()
             */
            const Key & AddKey(const std::string & path, const int new_key);
            /*! \overload AddKey()
             */
            const Key & AddKey(const std::string & path, const double new_key);

        protected:
            bool m_case_sensitive;
            Section m_root;
            std::string m_path;
            virtual void LoadConfig() = 0;

            Section & GetSection_unsafe(std::string const& path);
            Section & GetRoot_unsafe() {return m_root; };
            Key & GetKey_unsafe(std::string const& path);

        private:
            const Key & GetKey(const std::string & path);
            const Key & GetKey(const std::string & path, int default_val);
            const Key & GetKey(const std::string & path, double default_val);
            const Key & GetKey(const std::string & path, const std::string &default_val);

            //Utility function used to break the last word past the last / 
            //from the base path
            PathPair SplitPath(const std::string & path);

            //This function splits the path up like the function above, but also allows you to 
            //pass the path_elements vector in (for later traversal)
            PathPair SplitPathElements(const std::string & path, PathElementList & path_elements);

            //Utility function to determine if a given path is a leaf (i.e. it has no '/'s in it)
            bool IsLeaf(const std::string & path);
    };

}//end of namespace config

#endif //BL_CONFIG_HPP
