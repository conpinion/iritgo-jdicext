/**
 * This file is part of the Iritgo/JDICExt Library.
 *
 * Copyright (C) 2005-2011 Iritgo Technologies.
 * Copyright (C) 2003-2005 BueroByte GbR.
 *
 * Iritgo licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package de.iritgo.jdicext;


import java.util.LinkedList;
import java.util.List;


/**
 * This class provides access to platform system resources.
 */
public class JDICExt
{
	/** Singleton instance */
	private static JDICExt jdicExt;

	/** True if global key listeners are enabled */
	private boolean enabledGlobalKeyListener;

	/** Global key listeners */
	private List<GlobalKeyListener> listeners = new LinkedList<GlobalKeyListener> ();

	/**
	 * Create the singleton JDICExt instance.
	 */
	private JDICExt ()
	{
		if (System.getProperty ("os.name").indexOf ("Linux") != - 1)
		{
			System.loadLibrary ("iritgo-jdicext-linux");
		}
		else if (System.getProperty ("os.name").indexOf ("Windows") != - 1)
		{
			System.loadLibrary ("iritgo-jdicext-win32");
		}

		enabledGlobalKeyListener = true;
	}

	/**
	 * Get the singleton JDICExt instance.
	 *
	 * @return The JDICExt instance.
	 */
	public static JDICExt instance ()
	{
		if (jdicExt == null)
		{
			jdicExt = new JDICExt ();
		}

		return jdicExt;
	}

	/**
	 * Check if JDICExt is running under a supported operating system.
	 *
	 * @return True if the operating system is supported.
	 */
	public static boolean isSupportedOs ()
	{
		String osName = System.getProperty ("os.name");

		return osName.indexOf ("Linux") != - 1 || osName.indexOf ("Windows") != - 1;
	}

	public void installSystemHooks ()
	{
		if (isSupportedOs ())
		{
			nativeInstallSystemHooks ();
		}
	}

	public void removeSystemHooks ()
	{
		if (isSupportedOs ())
		{
			nativeRemoveSystemHooks ();
		}
	}

	public void addGlobalKeyListener (GlobalKeyListener listener)
	{
		listeners.add (listener);
	}

	public void removeGlobalKeyListener (GlobalKeyListener listener)
	{
		listeners.remove (listener);
	}

	public void fireGlobalKeyPressedEvent (GlobalKeyEvent event)
	{
		if (enabledGlobalKeyListener)
		{
			for (GlobalKeyListener listener : listeners)
			{
				listener.globalKeyPressed (event);
			}
		}
	}

	public void fireGlobalKeyReleasedEvent (GlobalKeyEvent event)
	{
		if (enabledGlobalKeyListener)
		{
			for (GlobalKeyListener listener : listeners)
			{
				listener.globalKeyReleased (event);
			}
		}
	}

	public void fireGlobalKeyTypedEvent (GlobalKeyEvent event)
	{
		if (enabledGlobalKeyListener)
		{
			for (GlobalKeyListener listener : listeners)
			{
				listener.globalKeyTyped (event);
			}
		}
	}

	public void setEnableGlobalKeyListener (boolean enable)
	{
		enabledGlobalKeyListener = enable;
	}

	public boolean isEnabledGlobalKeyListener ()
	{
		return enabledGlobalKeyListener;
	}

	public void globalKeyPressed (int time, int modifiers, int keyCode, int keyChar)
	{
		fireGlobalKeyPressedEvent (new GlobalKeyEvent ((long) time, modifiers, keyCode, (char) keyChar));
	}

	public void globalKeyReleased (int time, int modifiers, int keyCode, int keyChar)
	{
		fireGlobalKeyReleasedEvent (new GlobalKeyEvent ((long) time, modifiers, keyCode, (char) keyChar));

		fireGlobalKeyTypedEvent (new GlobalKeyEvent ((long) time, modifiers, keyCode, (char) keyChar));
	}

	private native void nativeInstallSystemHooks ();

	private native void nativeRemoveSystemHooks ();

	public native void copySelectedTextInActiveWindowToClipboard ();
}
