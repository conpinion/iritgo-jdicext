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

package de.iritgo.jdicext.test;


import de.iritgo.jdicext.JDICExt;
import java.awt.Toolkit;
import java.awt.datatransfer.Clipboard;
import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;


public class SelectionDemo
{
	public static void main(String[] args)
	{
		System.out.println("I give you ten seconds to select some text...");

		JDICExt.instance().installSystemHooks();

		try
		{
			Thread.sleep(10000);
		}
		catch (Exception x)
		{
		}

		JDICExt.instance().copySelectedTextInActiveWindowToClipboard();

		Clipboard clip = Toolkit.getDefaultToolkit().getSystemClipboard();
		Transferable contents = clip.getContents(null);

		if (contents != null)
		{
			try
			{
				System.out.println("Selected text: '" + (String) contents.getTransferData(DataFlavor.stringFlavor)
								+ "'");
			}
			catch (Exception x)
			{
				System.out.println("Error: " + x);
			}
		}

		JDICExt.instance().removeSystemHooks();
	}
}
