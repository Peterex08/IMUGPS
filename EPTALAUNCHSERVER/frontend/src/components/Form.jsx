import { useState } from "react";
import api from "../api";
import { useNavigate } from "react-router-dom";
import { ACCESS_TOKEN, REFRESH_TOKEN } from "../constants";
import "../styles/Form.css"
import LoadingIndicator from "./LoadingIndicator";

function Form({ route, method }) {
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [loading, setLoading] = useState(false);
    const navigate = useNavigate();

    const name = method === "login" ? "Login" : "Register";

    const handleSubmit = async (e) => {
        setLoading(true);
        e.preventDefault();

        try {
            const res = await api.post(route, { username, password })
            if (method === "login") {
                localStorage.setItem(ACCESS_TOKEN, res.data.access);
                localStorage.setItem(REFRESH_TOKEN, res.data.refresh);
                navigate("/")
            } else {
                navigate("/login")
            }
        } catch (error) {
            alert(error)
        } finally {
            setLoading(false)
        }
    };

    return (
        <div className="bg-gradient-to-t from-[#6cd4ff] to-[#152238] flex items-center justify-center h-screen">
            <div className="w-[80%] h-[90%] rounded-2xl flex flex-col">
                <img src="/eptalogobr.png" className="scale-50"/>
                <div className="">
                    <form onSubmit={handleSubmit} className="flex flex-col justify-center gap-y-8">
                        <h1 className="font-montserrat text-[#f2f2f2] text-2xl text-center">EPTA Telemetry Login</h1>
                        <input
                            className="font-montserrat font-bold text-xl bg-[#f2f2f2] rounded-full opacity-50 hover:opacity-70 pl-4 h-12 drop-shadow-lg"
                            type="text"
                            value={username}
                            onChange={(e) => setUsername(e.target.value)}
                            placeholder="Username"
                        />
                        <input
                            className=" font-montserrat font-bold text-xl bg-[#f2f2f2] rounded-full opacity-50 hover:opacity-70 pl-4 h-12 drop-shadow-lg"
                            type="password"
                            value={password}
                            onChange={(e) => setPassword(e.target.value)}
                            placeholder="Password"
                        />
                        {loading && <LoadingIndicator />}
                        <button className="font-montserrat cursor-pointer text-[#f2f2f2] font-bold bg-[#23395d] hover:bg-[#152238] rounded-full text-center mx-25 h-11 drop-shadow-lg" type="submit">
                            {name}
                        </button>
                    </form>
                </div>
            </div>
        </div>
    );
}

export default Form