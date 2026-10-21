using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
//
using System.Drawing;
using System.Runtime.InteropServices;

// Win32API: wrapper for selected Win32 API functions
// To compile:
//    csc /t:library /out:Win32API.dll Win32API.cs
//
//////////////////
// namespace to wrap Win32 API functions. Add them here as you need...
//
namespace god
{


    // These don't work, need to figure out how to fix it
    //[StructLayout(LayoutKind.Sequential)]
    //public struct IGFramework { };
    //[StructLayout(LayoutKind.Sequential)]
    //public struct IGGame { };

    public class GRuntime
    {


        // ... add more here
        [DllImport("GFrameworkDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int create3DViewport(int hwnd);

        // ... add more here
        
        // These don't work, need to figure out how to fix it
        //[DllImport("GFrameworkDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern IGFramework* getFramework(int hwnd);
        //[DllImport("GFrameworkDLL.dll", CallingConvention = CallingConvention.Cdecl)]
        //public static extern IGGame* GetGame(
        //    [MarshalAs(UnmanagedType.SysInt)]god.IGFramework* pFramework);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct POINT
    {
        public POINT(int xx, int yy) { x = xx; y = yy; }
        public int x;
        public int y;
        public override string ToString()
        {
            String s = String.Format("({0},{1})", x, y);
            return s;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct SIZE
    {
        public SIZE(int cxx, int cyy) { cx = cxx; cy = cyy; }
        public int cx;
        public int cy;
        public override string ToString()
        {
            String s = String.Format("({0},{1})", cx, cy);
            return s;
        }
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int left;
        public int top;
        public int right;
        public int bottom;
        public int Width() { return right - left; }
        public int Height() { return bottom - top; }
        public POINT TopLeft() { return new POINT(left, top); }
        public SIZE Size() { return new SIZE(Width(), Height()); }
        public override string ToString()
        {
            String s = String.Format("{0}x{1}", TopLeft(), Size());
            return s;
        }
    }

    public class CWinAPI
    { 
         [DllImport("user32.dll")]
        public static extern bool IsWindowVisible(int hwnd);

        [DllImport("user32.dll")]
        public static extern int GetWindowText(int hwnd,
           StringBuilder buf, int nMaxCount);

        [DllImport("user32.dll")]
        public static extern int SetWindowText(int hwnd,
           StringBuilder buf);

        [DllImport("user32.dll")]
        public static extern int GetClassName(int hwnd,
           [MarshalAs(UnmanagedType.LPStr)] StringBuilder buf,
           int nMaxCount);

        [DllImport("user32.dll")]
        public static extern int GetWindowRect(int hwnd, ref RECT rc);

        [DllImport("user32.dll")]
        // note the runtime knows how to marshal a Rectangle
        public static extern int GetWindowRect(int hwnd, ref System.Drawing.Rectangle rc);// Rectangle rc);   
    }
}