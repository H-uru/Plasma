/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

import SwiftUI

struct LoginView: View {
    
    @State var credentials = PLSLoginParameters()
    var client: PLSClient
    @Binding var isLoggedIn: Bool
    @State var showAlert: Bool = false
    @State var alertType: AlertTypes = .loggingIn
    @State var status = PLSServerStatus()
    @State var statusString = ""
    
    enum AlertTypes {
        case loginFailed
        case loggingIn
    }
    
    var body: some View {
        Form {
            Section {
                TextField("Username", text: $credentials.username)
                SecureField("Password", text: $credentials.password)
                Toggle("Remember Password", isOn: $credentials.rememberPassword)
            } header: {
                Text(statusString)
            } footer: {
                Button {
                    credentials.makeCurrent()
                    showAlert = true
                    alertType = .loggingIn
                    PLSLoginController.attemptLogin { result in
                        if result != 0 {
                            alertType = .loginFailed
                            showAlert = true
                            return
                        }
                        credentials.save()
                        client.initializeClient()
                        showAlert = false
                        isLoggedIn = true
                    }
                } label: {
                    Text("Login")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .padding(.top)
            }
        }
        .onAppear {
            credentials.load()
            status.load()
        }
        .onReceive(status.publisher(for: \.serverStatusString), perform: { output in
            statusString = output ?? ""
        })
        .alert(isPresented: $showAlert) {
            switch alertType {
            case .loginFailed:
                Alert(title: Text("Authentication Failed"), message: Text("Please try again."))
            case .loggingIn:
                Alert(title: Text("Logging in to URU. Please wait..."), dismissButton: nil)
            }
        }
    }
}

#Preview {
    var client = PLSClient()
    
    LoginView(client: client, isLoggedIn: .constant(true))
}
