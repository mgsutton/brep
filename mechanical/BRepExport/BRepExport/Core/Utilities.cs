using g3;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PADT.BRepExport.Core
{
  public static class Utilities
  {
    /// <summary>
    /// Splits an array into several smaller arrays.
    /// </summary>
    /// <typeparam name="T">The type of the array.</typeparam>
    /// <param name="array">The array to split.</param>
    /// <param name="size">The size of the smaller arrays.</param>
    /// <returns>An array containing smaller arrays.</returns>
    public static IEnumerable<T[]> SplitToArray<T>(this T[] array, int size)
    {
      for (var i = 0; i < (float)array.Length / size; i++)
      {
        yield return array.Skip(i * size).Take(size).Cast<T>().ToArray();
      }
    }

    /// <summary>
    /// This will take a list of facets from an ANSYS brep
    /// face and convert them into a form appropriate for
    /// DMesh
    /// </summary>
    /// <param name="ansysVertexPositions">
    /// The location of the vertices in the mesh.
    /// </param>
    /// <param name="ansysFacet">
    /// The ANSYS facets.
    /// </param>
    /// <param name="ansysVertexNormals">
    /// The vertex normals list from the ANSYS facets
    /// </param>
    /// <returns>A list in DMesh form.</returns>
    public static List<int>
        ConvertANSYSFacetListToDMesh(
        double[] ansysVertexPositions,
        int[] ansysFacet,
        double[] ansysVertexNormals
        )
    {
      Vector3d[] p = BufferUtil.ToVector3d(ansysVertexPositions);
      Vector3d[] n = BufferUtil.ToVector3d(ansysVertexNormals);
      List<int> facets = new List<int>(ansysFacet.Count() / 4);
      int i = 0;
      while (i < ansysFacet.Length)
      {
        if (ansysFacet[i] == 3)
        {
          int iv0 = ansysFacet[++i];
          int iv1 = ansysFacet[++i];
          int iv2 = ansysFacet[++i];
          Vector3d V0, V1, V2, N0;
          V0 = p[iv0];
          V1 = p[iv1];
          V2 = p[iv2];
          N0 = n[iv0];
          Vector3d edge1 = V1 - V0;
          Vector3d edge2 = V2 - V0;
          Vector3d normal = edge1.Cross(edge2);
          if (normal.Dot(N0) > 0)
          {
            facets.Add(iv0);
            facets.Add(iv1);
            facets.Add(iv2);
          }
          else
          {
            facets.Add(iv0);
            facets.Add(iv2);
            facets.Add(iv1);
          }
        }
        else
        {
          throw new InvalidDataException("Cannot convert an ANSYS facet list with a facet containing more than 3 vertices.");
        }
        i++;
      }
      return facets;
    }

    /// <summary>
    /// Break a list of items into chunks of a specific size
    /// </summary>
    public static IEnumerable<IEnumerable<T>>
        Chunk<T>(this IEnumerable<T> source, int chunksize)
    {
      while (source.Any())
      {
        yield return source.Take(chunksize);
        source = source.Skip(chunksize);
      }
    }
  }
}
