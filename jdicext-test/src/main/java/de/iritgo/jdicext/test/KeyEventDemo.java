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


import de.iritgo.jdicext.GlobalKeyAdapter;
import de.iritgo.jdicext.GlobalKeyEvent;
import de.iritgo.jdicext.JDICExt;
import java.awt.Event;


public class KeyEventDemo
{
	public static void main(String[] args)
	{
		JDICExt.instance().addGlobalKeyListener(new GlobalKeyAdapter()
		{
			public void globalKeyTyped(GlobalKeyEvent event)
			{
				System.out.println("Key typed:");

				if ((event.getModifiers() & Event.SHIFT_MASK) != 0)
				{
					System.out.println("\tSHIFT");
				}

				if ((event.getModifiers() & Event.CTRL_MASK) != 0)
				{
					System.out.println("\tCONTROL");
				}

				if ((event.getModifiers() & Event.ALT_MASK) != 0)
				{
					System.out.println("\tALT");
				}

				System.out.println("\tKey code: " + event.getKeyCode());

				if (event.getKeyChar() >= 32)
				{
					System.out.println("\tKey char: " + event.getKeyChar());
				}
				else
				{
					System.out.println("\tKey char: " + (int) event.getKeyChar());
				}
			}
		});

		JDICExt.instance().installSystemHooks();

		System.out.println("I give you ten seconds to try me...");

		try
		{
			Thread.sleep(10000);
		}
		catch (Exception x)
		{
		}

		JDICExt.instance().removeSystemHooks();
	}
}
