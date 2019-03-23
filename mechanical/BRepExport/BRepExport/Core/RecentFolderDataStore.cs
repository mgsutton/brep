using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Linq;

namespace PADT.BRepExport.Core
{
  /// <summary>
  /// This class is used to read from disk a registry.xml file stored in a
  /// directory on the user's machine that stores recent folders
  /// </summary>
  public static class RecentFolderDataStore
  {
    /// <summary>
    /// Retrieve the recently used folder for the given Key.
    /// </summary>
    /// <param name="Application">The name of the application.</param>
    /// <param name="Key">The particular key that is associated with this 
    ///                   folder.
    ///                   </param>
    /// <returns>A string path to the recent folder, or null if one does 
    ///          not exist in the registry.
    ///          </returns>
    public static string GetRecentFolder(string Application, string Key)
    {
      var appDataFolder =
  Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
      var thisAppFolder = Path.Combine(appDataFolder, Application);
      if (Directory.Exists(thisAppFolder))
      {
        var registry = Path.Combine(thisAppFolder, "registry.xml");
        if (File.Exists(registry))
        {
          XElement root = XElement.Load(registry);
          var dirPath = (from key in root.Elements("Key")
                         where (string)key.Attribute("name") == Key
                         select (string)key.Attribute("path")).
                         FirstOrDefault();
          return dirPath;
        }
      }
      return "";
    }

    /// <summary>
    /// This function will create and store the given data to disk so that
    /// it can be retrieved later
    /// </summary>
    /// <param name="Application">The application name.</param>
    /// <param name="Key">The particular key that is associated with this 
    ///                   folder.</param>
    /// <param name="Folder">The folder to store in the registry for the
    ///                       given key.</param>
    public static void StoreRecentFolder(
      string Application,
      string Key,
      string Folder)
    {
      var appDataFolder =
        Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
      var thisAppFolder = Path.Combine(appDataFolder, Application);
      Directory.CreateDirectory(thisAppFolder);
      var registry = Path.Combine(thisAppFolder, "registry.xml");
      XmlDocument doc = new XmlDocument();
      if (File.Exists(registry))
      {
        doc.Load(registry);
      }
      else
      {
        XmlNode docNode = doc.CreateXmlDeclaration("1.0", "UTF-8", null);
        doc.AppendChild(docNode);
        XmlNode KeysNode = doc.CreateElement("Keys");
        doc.AppendChild(KeysNode);
      }
      bool bFound = false;
      var KeyList = doc.SelectNodes("//Key");
      foreach (XmlNode node in KeyList)
      {
        var name = node.Attributes["name"];
        var path = node.Attributes["path"];
        if (name != null && name.Value == Key)
        {
          path.Value = Folder;
          bFound = true;
        }
      }
      if (!bFound)
      {
        XmlNode KeysNode = doc.SelectSingleNode("//Keys");
        XmlNode newKey = doc.CreateElement("Key");
        XmlAttribute nameAtt = doc.CreateAttribute("name");
        nameAtt.Value = Key;
        XmlAttribute pathAtt = doc.CreateAttribute("path");
        pathAtt.Value = Folder;
        newKey.Attributes.Append(nameAtt);
        newKey.Attributes.Append(pathAtt);
        KeysNode.AppendChild(newKey);
      }
      doc.Save(registry);
    }
  }
}
